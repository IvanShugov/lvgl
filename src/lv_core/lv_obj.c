/**
 * @file lv_base_obj.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_obj.h"
#include "lv_indev.h"
#include "lv_refr.h"
#include "lv_group.h"
#include "lv_disp.h"
#include "../lv_themes/lv_theme.h"
#include "../lv_draw/lv_draw.h"
#include "../lv_misc/lv_anim.h"
#include "../lv_misc/lv_task.h"
#include "../lv_misc/lv_fs.h"
#include "../lv_hal/lv_hal.h"
#include <stdint.h>
#include <string.h>
#include "../lv_misc/lv_gc.h"

#if defined(LV_GC_INCLUDE)
#   include LV_GC_INCLUDE
#endif /* LV_ENABLE_GC */

/*********************
 *      DEFINES
 *********************/
#define LV_OBJ_DEF_WIDTH  (LV_DPI)
#define LV_OBJ_DEF_HEIGHT  (2 * LV_DPI / 3)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void refresh_children_position(lv_obj_t * obj, lv_coord_t x_diff, lv_coord_t y_diff);
static void report_style_mod_core(void * style_p, lv_obj_t * obj);
static void refresh_children_style(lv_obj_t * obj);
static void delete_children(lv_obj_t * obj);
static bool lv_obj_design(lv_obj_t * obj, const  lv_area_t * mask_p, lv_design_mode_t mode);
static lv_res_t lv_obj_signal(lv_obj_t * obj, lv_signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool lv_initialized = false;
static lv_obj_t * obj_act_event;        /*Stores the which event is currently being executed*/
static bool obj_act_event_deleted;      /*Shows that the object was deleted in the event function*/
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Init. the 'lv' library.
 */
void lv_init(void)
{
    /* Do nothing if already initialized */
    if (lv_initialized) {
        LV_LOG_WARN("lv_init: already inited");
        return;
    }

    LV_LOG_TRACE("lv_init started");

    /*Initialize the lv_misc modules*/
    lv_mem_init();
    lv_task_init();

#if LV_USE_FILESYSTEM
    lv_fs_init();
#endif

    lv_font_init();
#if LV_USE_ANIMATION
    lv_anim_init();
#endif

#if LV_USE_GROUP
    lv_group_init();
#endif

    /*Init. the sstyles*/
    lv_style_init();

    /*Initialize the screen refresh system*/
    lv_refr_init();

    lv_ll_init(&LV_GC_ROOT(_lv_disp_ll), sizeof(lv_disp_t));
    lv_ll_init(&LV_GC_ROOT(_lv_indev_ll), sizeof(lv_indev_t));


#if LV_INDEV_READ_PERIOD != 0
    /*Init the input device handling*/
    lv_indev_init();
#endif

    lv_initialized = true;
    LV_LOG_INFO("lv_init ready");
}

/*--------------------
 * Create and delete
 *-------------------*/

/**
 * Create a basic object
 * @param parent pointer to a parent object.
 *                  If NULL then a screen will be created
 * @param copy pointer to a base object, if not NULL then the new object will be copied from it
 * @return pointer to the new object
 */
lv_obj_t * lv_obj_create(lv_obj_t * parent, const  lv_obj_t * copy)
{

    lv_obj_t * new_obj = NULL;
    /*Create a screen if the parent is NULL*/
    if(parent == NULL) {
        LV_LOG_TRACE("Screen create started");
        lv_disp_t * disp = lv_disp_get_default();
        if(!disp) {
            LV_LOG_WARN("lv_obj_create: not display created to so far. No place to assign the new screen");
            return NULL;
        }

        new_obj = lv_ll_ins_head(&disp->scr_ll);
        lv_mem_assert(new_obj);
        if(new_obj == NULL) return NULL;

        new_obj->par = NULL; /*Screens has no a parent*/
        lv_ll_init(&(new_obj->child_ll), sizeof(lv_obj_t));

        /*Set coordinates to full screen size*/
        new_obj->coords.x1 = 0;
        new_obj->coords.y1 = 0;
        new_obj->coords.x2 = lv_disp_get_hor_res(NULL) - 1;
        new_obj->coords.y2 = lv_disp_get_ver_res(NULL) - 1;
        new_obj->ext_size = 0;

        /*Init realign*/
#if LV_OBJ_REALIGN
        new_obj->realign.align = LV_ALIGN_CENTER;
        new_obj->realign.xofs = 0;
        new_obj->realign.yofs = 0;
        new_obj->realign.base = NULL;
        new_obj->realign.auto_realign = 0;
#endif

        /*Set the default styles*/
        lv_theme_t * th = lv_theme_get_current();
        if(th) {
            new_obj->style_p = th->style.bg;
        } else {
            new_obj->style_p = &lv_style_scr;
        }
        /*Set the callbacks*/
        lv_obj_set_signal_cb(new_obj, lv_obj_signal);
        lv_obj_set_design_cb(new_obj, lv_obj_design);
        new_obj->event_cb = NULL;

        /*Init. user date*/
#if LV_USE_USER_DATA_SINGLE
        memset(&new_obj->user_data, 0, sizeof(lv_obj_user_data_t));
#endif
#if LV_USE_USER_DATA_MULTI
        memset(&new_obj->event_user_data, 0, sizeof(lv_obj_user_data_t));
        memset(&new_obj->signal_user_data, 0, sizeof(lv_obj_user_data_t));
        memset(&new_obj->design_user_data, 0, sizeof(lv_obj_user_data_t));
#endif

#if LV_USE_GROUP
        new_obj->group_p = NULL;
#endif
        /*Set attributes*/
        new_obj->click = 0;
        new_obj->drag = 0;
        new_obj->drag_throw = 0;
        new_obj->drag_parent = 0;
        new_obj->hidden = 0;
        new_obj->top = 0;
        new_obj->protect = LV_PROTECT_NONE;
        new_obj->opa_scale_en = 0;
        new_obj->opa_scale = LV_OPA_COVER;
        new_obj->parent_event = 0;

        new_obj->ext_attr = NULL;

        LV_LOG_INFO("Screen create ready");
    }
    /*parent != NULL create normal obj. on a parent*/
    else {
        LV_LOG_TRACE("Object create started");

        new_obj = lv_ll_ins_head(&(parent)->child_ll);
        lv_mem_assert(new_obj);
        if(new_obj == NULL) return NULL;


        new_obj->par = parent; /*Set the parent*/
        lv_ll_init(&(new_obj->child_ll), sizeof(lv_obj_t));

        /*Set coordinates left top corner of parent*/
        new_obj->coords.x1 = parent->coords.x1;
        new_obj->coords.y1 = parent->coords.y1;
        new_obj->coords.x2 = parent->coords.x1 +
                             LV_OBJ_DEF_WIDTH;
        new_obj->coords.y2 = parent->coords.y1 +
                             LV_OBJ_DEF_HEIGHT;
        new_obj->ext_size = 0;

        /*Init realign*/
#if LV_OBJ_REALIGN
        new_obj->realign.align = LV_ALIGN_CENTER;
        new_obj->realign.xofs = 0;
        new_obj->realign.yofs = 0;
        new_obj->realign.base = NULL;
        new_obj->realign.auto_realign = 0;
#endif
        /*Set appearance*/
        lv_theme_t * th = lv_theme_get_current();
        if(th) {
            new_obj->style_p = th->style.panel;
        } else {
            new_obj->style_p = &lv_style_plain_color;
        }

        /*Set the callbacks*/
        lv_obj_set_signal_cb(new_obj, lv_obj_signal);
        lv_obj_set_design_cb(new_obj, lv_obj_design);
        new_obj->event_cb = NULL;

        /*Init. user date*/
#if LV_USE_USER_DATA_SINGLE
        memset(&new_obj->user_data, 0, sizeof(lv_obj_user_data_t));
#endif
#if LV_USE_USER_DATA_MULTI
        memset(&new_obj->event_user_data, 0, sizeof(lv_obj_user_data_t));
        memset(&new_obj->signal_user_data, 0, sizeof(lv_obj_user_data_t));
        memset(&new_obj->design_user_data, 0, sizeof(lv_obj_user_data_t));
#endif

#if LV_USE_GROUP
        new_obj->group_p = NULL;
#endif

        /*Set attributes*/
        new_obj->click = 1;
        new_obj->drag = 0;
        new_obj->drag_throw = 0;
        new_obj->drag_parent = 0;
        new_obj->hidden = 0;
        new_obj->top = 0;
        new_obj->protect = LV_PROTECT_NONE;
        new_obj->opa_scale = LV_OPA_COVER;
        new_obj->opa_scale_en = 0;
        new_obj->parent_event = 0;

        new_obj->ext_attr = NULL;
    }

    /*Copy the attributes if required*/
    if(copy != NULL) {
        lv_area_copy(&new_obj->coords, &copy->coords);
        new_obj->ext_size = copy->ext_size;

        /*Set free data*/
#if LV_USE_USER_DATA_SINGLE
        memcpy(&new_obj->user_data, &copy->user_data, sizeof(lv_obj_user_data_t));
#endif
#if LV_USE_USER_DATA_MULTI
        memcpy(&new_obj->event_user_data, &copy->event_user_data, sizeof(lv_obj_user_data_t));
        memcpy(&new_obj->signal_user_data, &copy->signal_user_data, sizeof(lv_obj_user_data_t));
        memcpy(&new_obj->design_user_data, &copy->design_user_data, sizeof(lv_obj_user_data_t));
#endif

        /*Copy realign*/
#if LV_OBJ_REALIGN
        new_obj->realign.align = copy->realign.align;
        new_obj->realign.xofs = copy->realign.xofs;
        new_obj->realign.yofs = copy->realign.yofs;
        new_obj->realign.base = copy->realign.base;
        new_obj->realign.auto_realign = copy->realign.auto_realign;
#endif

        /*Only copy the `event_cb`. `signal_cb` and `design_cb` will be copied the the derived object type (e.g. `lv_btn`)*/
        new_obj-> event_cb = copy->event_cb;

        /*Copy attributes*/
        new_obj->click = copy->click;
        new_obj->drag = copy->drag;
        new_obj->drag_throw = copy->drag_throw;
        new_obj->drag_parent = copy->drag_parent;
        new_obj->hidden = copy->hidden;
        new_obj->top = copy->top;
        new_obj->parent_event = copy->parent_event;

        new_obj->opa_scale_en = copy->opa_scale_en;
        new_obj->protect = copy->protect;
        new_obj->opa_scale = copy->opa_scale;

        new_obj->style_p = copy->style_p;

#if LV_USE_GROUP
        /*Add to the same group*/
        if(copy->group_p != NULL) {
            lv_group_add_obj(copy->group_p, new_obj);
        }
#endif

        lv_obj_set_pos(new_obj, lv_obj_get_x(copy), lv_obj_get_y(copy));

        LV_LOG_INFO("Object create ready");
    }


    /*Send a signal to the parent to notify it about the new child*/
    if(parent != NULL) {
        parent->signal_cb(parent, LV_SIGNAL_CHILD_CHG, new_obj);

        /*Invalidate the area if not screen created*/
        lv_obj_invalidate(new_obj);
    }

    return new_obj;
}

/**
 * Delete 'obj' and all of its children
 * @param obj pointer to an object to delete
 * @return LV_RES_INV because the object is deleted
 */
lv_res_t lv_obj_del(lv_obj_t * obj)
{
    lv_obj_invalidate(obj);

    if(obj_act_event == obj && obj_act_event_deleted == false) obj_act_event_deleted = true;

    /*Delete from the group*/
#if LV_USE_GROUP
    bool was_focused = false;

    if(obj->group_p) {
        if(lv_group_get_focused(obj->group_p) == obj) was_focused = true;
        lv_group_remove_obj(obj);
    }
#endif

    /*Remove the animations from this object*/
#if LV_USE_ANIMATION
    lv_anim_del(obj, NULL);
#endif

    /*Recursively delete the children*/
    lv_obj_t * i;
    lv_obj_t * i_next;
    i = lv_ll_get_head(&(obj->child_ll));
    while(i != NULL) {
        /*Get the next object before delete this*/
        i_next = lv_ll_get_next(&(obj->child_ll), i);

        /*Call the recursive del to the child too*/
        delete_children(i);

        /*Set i to the next node*/
        i = i_next;
    }

    /*Remove the object from parent's children list*/
    lv_obj_t * par = lv_obj_get_parent(obj);
    if(par == NULL) { /*It is a screen*/
        lv_disp_t * d = lv_obj_get_disp(obj);
        lv_ll_rem(&d->scr_ll, obj);
    } else {
        lv_ll_rem(&(par->child_ll), obj);
    }

    /* Reset all input devices if
     * the object to delete is used*/
    lv_indev_t * indev = lv_indev_next(NULL);
    while(indev) {
        if(indev->proc.types.pointer.act_obj == obj || indev->proc.types.pointer.last_obj == obj) {
            lv_indev_reset(indev);
        }

#if LV_USE_GROUP
        if(was_focused) {
            lv_indev_reset(indev);
        }
#endif
        indev = lv_indev_next(indev);
    }

    /* All children deleted.
     * Now clean up the object specific data*/
    obj->signal_cb(obj, LV_SIGNAL_CLEANUP, NULL);

    /*Delete the base objects*/
    if(obj->ext_attr != NULL)  lv_mem_free(obj->ext_attr);
    lv_mem_free(obj); /*Free the object itself*/

    /*Send a signal to the parent to notify it about the child delete*/
    if(par != NULL) {
        par->signal_cb(par, LV_SIGNAL_CHILD_CHG, NULL);
    }

    return LV_RES_INV;
}

/**
 * Delete all children of an object
 * @param obj pointer to an object
 */
void lv_obj_clean(lv_obj_t * obj)
{
    lv_obj_t * child = lv_obj_get_child(obj, NULL);
    lv_obj_t * child_next;
    while(child) {
        /* Read the next child before deleting the current
         * because the next couldn't be read from a deleted (invalid) node*/
        child_next = lv_obj_get_child(obj, child);
        lv_obj_del(child);
        child = child_next;
    }
}

/**
 * Mark the object as invalid therefore its current position will be redrawn by 'lv_refr_task'
 * @param obj pointer to an object
 */
void lv_obj_invalidate(const lv_obj_t * obj)
{
    if(lv_obj_get_hidden(obj)) return;

    /*Invalidate the object only if it belongs to the 'LV_GC_ROOT(_lv_act_scr)'*/
    lv_obj_t * obj_scr = lv_obj_get_screen(obj);
    lv_disp_t * disp = lv_obj_get_disp(obj_scr);
    if(obj_scr == lv_disp_get_scr_act(disp) ||
            obj_scr == lv_disp_get_layer_top(disp)||
            obj_scr == lv_disp_get_layer_sys(disp)) {
        /*Truncate recursively to the parents*/
        lv_area_t area_trunc;
        lv_obj_t * par = lv_obj_get_parent(obj);
        bool union_ok = true;
        /*Start with the original coordinates*/
        lv_coord_t ext_size = obj->ext_size;
        lv_area_copy(&area_trunc, &obj->coords);
        area_trunc.x1 -= ext_size;
        area_trunc.y1 -= ext_size;
        area_trunc.x2 += ext_size;
        area_trunc.y2 += ext_size;

        /*Check through all parents*/
        while(par != NULL) {
            union_ok = lv_area_intersect(&area_trunc, &area_trunc, &par->coords);
            if(union_ok == false) break;        /*If no common parts with parent break;*/
            if(lv_obj_get_hidden(par)) return;  /*If the parent is hidden then the child is hidden and won't be drawn*/

            par = lv_obj_get_parent(par);
        }

        if(union_ok) lv_inv_area(disp, &area_trunc);
    }
}


/*=====================
 * Setter functions
 *====================*/

/*--------------------
 * Parent/children set
 *--------------------*/

/**
 * Set a new parent for an object. Its relative position will be the same.
 * @param obj pointer to an object. Can't be a screen.
 * @param parent pointer to the new parent object. (Can't be NULL)
 */
void lv_obj_set_parent(lv_obj_t * obj, lv_obj_t * parent)
{
    if(obj->par == NULL) {
        LV_LOG_WARN("Can't set the parent of a screen");
        return;
    }

    if(parent == NULL) {
        LV_LOG_WARN("Can't set parent == NULL to an object");
        return;
    }


    lv_obj_invalidate(obj);

    lv_point_t old_pos;
    old_pos.x = lv_obj_get_x(obj);
    old_pos.y = lv_obj_get_y(obj);

    lv_obj_t * old_par = obj->par;

    lv_ll_chg_list(&obj->par->child_ll, &parent->child_ll, obj);
    obj->par = parent;
    lv_obj_set_pos(obj, old_pos.x, old_pos.y);

    /*Notify the original parent because one of its children is lost*/
    old_par->signal_cb(old_par, LV_SIGNAL_CHILD_CHG, NULL);

    /*Notify the new parent about the child*/
    parent->signal_cb(parent, LV_SIGNAL_CHILD_CHG, obj);

    lv_obj_invalidate(obj);
}

/*--------------------
 * Coordinate set
 * ------------------*/

/**
 * Set relative the position of an object (relative to the parent)
 * @param obj pointer to an object
 * @param x new distance from the left side of the parent
 * @param y new distance from the top of the parent
 */
void lv_obj_set_pos(lv_obj_t * obj, lv_coord_t x, lv_coord_t y)
{
    /*Convert x and y to absolute coordinates*/
    lv_obj_t * par = obj->par;
    x = x + par->coords.x1;
    y = y + par->coords.y1;

    /*Calculate and set the movement*/
    lv_point_t diff;
    diff.x =  x - obj->coords.x1;
    diff.y =  y - obj->coords.y1;

    /* Do nothing if the position is not changed */
    /* It is very important else recursive positioning can
     * occur without position change*/
    if(diff.x == 0 && diff.y == 0) return;

    /*Invalidate the original area*/
    lv_obj_invalidate(obj);

    /*Save the original coordinates*/
    lv_area_t ori;
    lv_obj_get_coords(obj, &ori);

    obj->coords.x1 += diff.x;
    obj->coords.y1 += diff.y;
    obj->coords.x2 += diff.x;
    obj->coords.y2 += diff.y;

    refresh_children_position(obj, diff.x, diff.y);

    /*Inform the object about its new coordinates*/
    obj->signal_cb(obj, LV_SIGNAL_CORD_CHG, &ori);

    /*Send a signal to the parent too*/
    par->signal_cb(par, LV_SIGNAL_CHILD_CHG, obj);

    /*Invalidate the new area*/
    lv_obj_invalidate(obj);
}


/**
 * Set the x coordinate of a object
 * @param obj pointer to an object
 * @param x new distance from the left side from the parent
 */
void lv_obj_set_x(lv_obj_t * obj, lv_coord_t x)
{
    lv_obj_set_pos(obj, x, lv_obj_get_y(obj));
}


/**
 * Set the y coordinate of a object
 * @param obj pointer to an object
 * @param y new distance from the top of the parent
 */
void lv_obj_set_y(lv_obj_t * obj, lv_coord_t y)
{
    lv_obj_set_pos(obj, lv_obj_get_x(obj), y);
}

/**
 * Set the size of an object
 * @param obj pointer to an object
 * @param w new width
 * @param h new height
 */
void lv_obj_set_size(lv_obj_t * obj, lv_coord_t w, lv_coord_t h)
{

    /* Do nothing if the size is not changed */
    /* It is very important else recursive resizing can
     * occur without size change*/
    if(lv_obj_get_width(obj) == w && lv_obj_get_height(obj) == h) {
        return;
    }

    /*Invalidate the original area*/
    lv_obj_invalidate(obj);

    /*Save the original coordinates*/
    lv_area_t ori;
    lv_obj_get_coords(obj, &ori);

    //Set the length and height
    obj->coords.x2 = obj->coords.x1 + w - 1;
    obj->coords.y2 = obj->coords.y1 + h - 1;


    /*Send a signal to the object with its new coordinates*/
    obj->signal_cb(obj, LV_SIGNAL_CORD_CHG, &ori);

    /*Send a signal to the parent too*/
    lv_obj_t * par = lv_obj_get_parent(obj);
    if(par != NULL) par->signal_cb(par, LV_SIGNAL_CHILD_CHG, obj);

    /*Tell the children the parent's size has changed*/
    lv_obj_t * i;
    LV_LL_READ(obj->child_ll, i) {
       i->signal_cb(i, LV_SIGNAL_PARENT_SIZE_CHG, NULL);
    }

    /*Invalidate the new area*/
    lv_obj_invalidate(obj);

    /*Automatically realign the object if required*/
#if LV_OBJ_REALIGN
    if(obj->realign.auto_realign) lv_obj_realign(obj);
#endif
}

/**
 * Set the width of an object
 * @param obj pointer to an object
 * @param w new width
 */
void lv_obj_set_width(lv_obj_t * obj, lv_coord_t w)
{
    lv_obj_set_size(obj, w, lv_obj_get_height(obj));
}

/**
 * Set the height of an object
 * @param obj pointer to an object
 * @param h new height
 */
void lv_obj_set_height(lv_obj_t * obj, lv_coord_t h)
{
    lv_obj_set_size(obj, lv_obj_get_width(obj), h);
}

/**
 * Align an object to an other object.
 * @param obj pointer to an object to align
 * @param base pointer to an object (if NULL the parent is used). 'obj' will be aligned to it.
 * @param align type of alignment (see 'lv_align_t' enum)
 * @param x_mod x coordinate shift after alignment
 * @param y_mod y coordinate shift after alignment
 */
void lv_obj_align(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align, lv_coord_t x_mod, lv_coord_t y_mod)
{
    lv_coord_t new_x = lv_obj_get_x(obj);
    lv_coord_t new_y = lv_obj_get_y(obj);

    if(base == NULL) {
        base = lv_obj_get_parent(obj);
    }

    switch(align) {
        case LV_ALIGN_CENTER:
            new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
            new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
            break;

        case LV_ALIGN_IN_TOP_LEFT:
            new_x = 0;
            new_y = 0;
            break;
        case LV_ALIGN_IN_TOP_MID:
            new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
            new_y = 0;
            break;

        case LV_ALIGN_IN_TOP_RIGHT:
            new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
            new_y = 0;
            break;

        case LV_ALIGN_IN_BOTTOM_LEFT:
            new_x = 0;
            new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
            break;
        case LV_ALIGN_IN_BOTTOM_MID:
            new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
            new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
            break;

        case LV_ALIGN_IN_BOTTOM_RIGHT:
            new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
            new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
            break;

        case LV_ALIGN_IN_LEFT_MID:
            new_x = 0;
            new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
            break;

        case LV_ALIGN_IN_RIGHT_MID:
            new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
            new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
            break;

        case LV_ALIGN_OUT_TOP_LEFT:
            new_x = 0;
            new_y = -lv_obj_get_height(obj);
            break;

        case LV_ALIGN_OUT_TOP_MID:
            new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
            new_y = - lv_obj_get_height(obj);
            break;

        case LV_ALIGN_OUT_TOP_RIGHT:
            new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
            new_y = - lv_obj_get_height(obj);
            break;

        case LV_ALIGN_OUT_BOTTOM_LEFT:
            new_x = 0;
            new_y = lv_obj_get_height(base);
            break;

        case LV_ALIGN_OUT_BOTTOM_MID:
            new_x = lv_obj_get_width(base) / 2 - lv_obj_get_width(obj) / 2;
            new_y = lv_obj_get_height(base);
            break;

        case LV_ALIGN_OUT_BOTTOM_RIGHT:
            new_x = lv_obj_get_width(base) - lv_obj_get_width(obj);
            new_y = lv_obj_get_height(base);
            break;

        case LV_ALIGN_OUT_LEFT_TOP:
            new_x = - lv_obj_get_width(obj);
            new_y = 0;
            break;

        case LV_ALIGN_OUT_LEFT_MID:
            new_x = - lv_obj_get_width(obj);
            new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
            break;

        case LV_ALIGN_OUT_LEFT_BOTTOM:
            new_x = - lv_obj_get_width(obj);
            new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
            break;

        case LV_ALIGN_OUT_RIGHT_TOP:
            new_x = lv_obj_get_width(base);
            new_y = 0;
            break;

        case LV_ALIGN_OUT_RIGHT_MID:
            new_x = lv_obj_get_width(base);
            new_y = lv_obj_get_height(base) / 2 - lv_obj_get_height(obj) / 2;
            break;

        case LV_ALIGN_OUT_RIGHT_BOTTOM:
            new_x = lv_obj_get_width(base);
            new_y = lv_obj_get_height(base) - lv_obj_get_height(obj);
            break;
    }

    /*Bring together the coordination system of base and obj*/
    lv_obj_t * par = lv_obj_get_parent(obj);
    lv_coord_t base_abs_x = base->coords.x1;
    lv_coord_t base_abs_y = base->coords.y1;
    lv_coord_t par_abs_x = par->coords.x1;
    lv_coord_t par_abs_y = par->coords.y1;
    new_x += x_mod + base_abs_x;
    new_y += y_mod + base_abs_y;
    new_x -= par_abs_x;
    new_y -= par_abs_y;

    lv_obj_set_pos(obj, new_x, new_y);

#if LV_OBJ_REALIGN
    /*Save the last align parameters to use them in `lv_obj_realign`*/
    obj->realign.align = align;
    obj->realign.xofs = x_mod;
    obj->realign.yofs = y_mod;
    obj->realign.base = base;
    obj->realign.origo_align = 0;
#endif
}

/**
 * Align an object to an other object.
 * @param obj pointer to an object to align
 * @param base pointer to an object (if NULL the parent is used). 'obj' will be aligned to it.
 * @param align type of alignment (see 'lv_align_t' enum)
 * @param x_mod x coordinate shift after alignment
 * @param y_mod y coordinate shift after alignment
 */
void lv_obj_align_origo(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align, lv_coord_t x_mod, lv_coord_t y_mod)
{
    lv_coord_t new_x = lv_obj_get_x(obj);
    lv_coord_t new_y = lv_obj_get_y(obj);

    lv_coord_t obj_w_half =  lv_obj_get_width(obj) / 2;
    lv_coord_t obj_h_half = lv_obj_get_height(obj) / 2;

    if(base == NULL) {
        base = lv_obj_get_parent(obj);
    }

    switch(align) {
        case LV_ALIGN_CENTER:
            new_x = lv_obj_get_width(base) / 2 - obj_w_half;
            new_y = lv_obj_get_height(base) / 2 - obj_h_half;
            break;

        case LV_ALIGN_IN_TOP_LEFT:
            new_x = -obj_w_half;
            new_y = -obj_h_half;
            break;
        case LV_ALIGN_IN_TOP_MID:
            new_x = lv_obj_get_width(base) / 2 - obj_w_half;
            new_y = -obj_h_half;
            break;

        case LV_ALIGN_IN_TOP_RIGHT:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = -obj_h_half;
            break;

        case LV_ALIGN_IN_BOTTOM_LEFT:
            new_x = -obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;
        case LV_ALIGN_IN_BOTTOM_MID:
            new_x = lv_obj_get_width(base) / 2 - obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;

        case LV_ALIGN_IN_BOTTOM_RIGHT:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;

        case LV_ALIGN_IN_LEFT_MID:
            new_x = -obj_w_half;
            new_y = lv_obj_get_height(base) / 2 - obj_h_half;
            break;

        case LV_ALIGN_IN_RIGHT_MID:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = lv_obj_get_height(base) / 2 - obj_h_half;
            break;

        case LV_ALIGN_OUT_TOP_LEFT:
            new_x = -obj_w_half;
            new_y = -obj_h_half;
            break;

        case LV_ALIGN_OUT_TOP_MID:
            new_x = lv_obj_get_width(base) / 2 - obj_w_half;
            new_y = -obj_h_half;
            break;

        case LV_ALIGN_OUT_TOP_RIGHT:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = - obj_h_half;
            break;

        case LV_ALIGN_OUT_BOTTOM_LEFT:
            new_x = -obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;

        case LV_ALIGN_OUT_BOTTOM_MID:
            new_x = lv_obj_get_width(base) / 2 - obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;

        case LV_ALIGN_OUT_BOTTOM_RIGHT:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;

        case LV_ALIGN_OUT_LEFT_TOP:
            new_x = - obj_w_half ;
            new_y = - obj_h_half;
            break;

        case LV_ALIGN_OUT_LEFT_MID:
            new_x = - obj_w_half;
            new_y = lv_obj_get_height(base) / 2 - obj_h_half;
            break;

        case LV_ALIGN_OUT_LEFT_BOTTOM:
            new_x = - obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;

        case LV_ALIGN_OUT_RIGHT_TOP:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = -obj_h_half;
            break;

        case LV_ALIGN_OUT_RIGHT_MID:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = lv_obj_get_height(base) / 2 - obj_h_half;
            break;

        case LV_ALIGN_OUT_RIGHT_BOTTOM:
            new_x = lv_obj_get_width(base) - obj_w_half;
            new_y = lv_obj_get_height(base) - obj_h_half;
            break;
    }

    /*Bring together the coordination system of base and obj*/
    lv_obj_t * par = lv_obj_get_parent(obj);
    lv_coord_t base_abs_x = base->coords.x1;
    lv_coord_t base_abs_y = base->coords.y1;
    lv_coord_t par_abs_x = par->coords.x1;
    lv_coord_t par_abs_y = par->coords.y1;
    new_x += x_mod + base_abs_x;
    new_y += y_mod + base_abs_y;
    new_x -= par_abs_x;
    new_y -= par_abs_y;

    lv_obj_set_pos(obj, new_x, new_y);

#if LV_OBJ_REALIGN
    /*Save the last align parameters to use them in `lv_obj_realign`*/
    obj->realign.align = align;
    obj->realign.xofs = x_mod;
    obj->realign.yofs = y_mod;
    obj->realign.base = base;
    obj->realign.origo_align = 1;
#endif
}

/**
 * Realign the object based on the last `lv_obj_align` parameters.
 * @param obj pointer to an object
 */
void lv_obj_realign(lv_obj_t * obj)
{
#if LV_OBJ_REALIGN
    if(obj->realign.origo_align) lv_obj_align_origo(obj, obj->realign.base, obj->realign.align, obj->realign.xofs, obj->realign.yofs);
    else lv_obj_align(obj, obj->realign.base, obj->realign.align, obj->realign.xofs, obj->realign.yofs);
#else
    (void) obj;
    LV_LOG_WARN("lv_obj_realaign: no effect because LV_OBJ_REALIGN = 0");
#endif
}

/**
 * Enable the automatic realign of the object when its size has changed based on the last `lv_obj_align` parameters.
 * @param obj pointer to an object
 * @param en true: enable auto realign; false: disable auto realign
 */
void lv_obj_set_auto_realign(lv_obj_t * obj, bool en)
{
#if LV_OBJ_REALIGN
    obj->realign.auto_realign = en ? 1 : 0;
#else
    (void) obj;
    (void) en;
    LV_LOG_WARN("lv_obj_set_auto_realign: no effect because LV_OBJ_REALIGN = 0");
#endif
}

/*---------------------
 * Appearance set
 *--------------------*/

/**
 * Set a new style for an object
 * @param obj pointer to an object
 * @param style_p pointer to the new style
 */
void lv_obj_set_style(lv_obj_t * obj, lv_style_t * style)
{
    obj->style_p = style;

    /*Send a signal about style change to every children with NULL style*/
    refresh_children_style(obj);

    /*Notify the object about the style change too*/
    lv_obj_refresh_style(obj);
}

/**
 * Notify an object about its style is modified
 * @param obj pointer to an object
 */
void lv_obj_refresh_style(lv_obj_t * obj)
{
    lv_obj_invalidate(obj);
    obj->signal_cb(obj, LV_SIGNAL_STYLE_CHG, NULL);
    lv_obj_invalidate(obj);

}

/**
 * Notify all object if a style is modified
 * @param style pointer to a style. Only the objects with this style will be notified
 *               (NULL to notify all objects)
 */
void lv_obj_report_style_mod(lv_style_t * style)
{
    lv_disp_t * d = lv_disp_get_next(NULL);

    while(d) {
        lv_obj_t * i;
        LV_LL_READ(d->scr_ll, i) {
            if(i->style_p == style || style == NULL) {
                lv_obj_refresh_style(i);
            }

            report_style_mod_core(style, i);
        }
        d = lv_disp_get_next(d);
    }
}

/*-----------------
 * Attribute set
 *----------------*/

/**
 * Hide an object. It won't be visible and clickable.
 * @param obj pointer to an object
 * @param en true: hide the object
 */
void lv_obj_set_hidden(lv_obj_t * obj, bool en)
{
    if(!obj->hidden) lv_obj_invalidate(obj);    /*Invalidate when not hidden (hidden objects are ignored) */

    obj->hidden = en == false ? 0 : 1;

    if(!obj->hidden) lv_obj_invalidate(obj);    /*Invalidate when not hidden (hidden objects are ignored) */

    lv_obj_t * par = lv_obj_get_parent(obj);
    par->signal_cb(par, LV_SIGNAL_CHILD_CHG, obj);

}

/**
 * Enable or disable the clicking of an object
 * @param obj pointer to an object
 * @param en true: make the object clickable
 */
void lv_obj_set_click(lv_obj_t * obj, bool en)
{
    obj->click = (en == true ? 1 : 0);
}

/**
 * Enable to bring this object to the foreground if it
 * or any of its children is clicked
 * @param obj pointer to an object
 * @param en true: enable the auto top feature
 */
void lv_obj_set_top(lv_obj_t * obj, bool en)
{
    obj->top = (en == true ? 1 : 0);
}

/**
 * Enable the dragging of an object
 * @param obj pointer to an object
 * @param en true: make the object dragable
 */
void lv_obj_set_drag(lv_obj_t * obj, bool en)
{
    if(en == true) lv_obj_set_click(obj, true);     /*Drag is useless without enabled clicking*/
    obj->drag = (en == true ? 1 : 0);
}

/**
 * Enable the throwing of an object after is is dragged
 * @param obj pointer to an object
 * @param en true: enable the drag throw
 */
void lv_obj_set_drag_throw(lv_obj_t * obj, bool en)
{
    obj->drag_throw = (en == true ? 1 : 0);
}

/**
 * Enable to use parent for drag related operations.
 * If trying to drag the object the parent will be moved instead
 * @param obj pointer to an object
 * @param en true: enable the 'drag parent' for the object
 */
void lv_obj_set_drag_parent(lv_obj_t * obj, bool en)
{
    obj->drag_parent = (en == true ? 1 : 0);
}

/**
 * Propagate the events to the parent too
 * @param obj pointer to an object
 * @param en true: enable the event propagation
 */
void lv_obj_set_parent_event(lv_obj_t * obj, bool en)
{
    obj->parent_event = (en == true ? 1 : 0);
}

/**
 * Set the opa scale enable parameter (required to set opa_scale with `lv_obj_set_opa_scale()`)
 * @param obj pointer to an object
 * @param en true: opa scaling is enabled for this object and all children; false: no opa scaling
 */
void lv_obj_set_opa_scale_enable(lv_obj_t * obj, bool en)
{
    obj->opa_scale_en =  en ? 1 : 0;
}

/**
 * Set the opa scale of an object
 * @param obj pointer to an object
 * @param opa_scale a factor to scale down opacity [0..255]
 */
void lv_obj_set_opa_scale(lv_obj_t * obj, lv_opa_t opa_scale)
{
    obj->opa_scale = opa_scale;
    lv_obj_invalidate(obj);
}

/**
 * Set a bit or bits in the protect filed
 * @param obj pointer to an object
 * @param prot 'OR'-ed values from `lv_protect_t`
 */
void lv_obj_set_protect(lv_obj_t * obj, uint8_t prot)
{
    obj->protect |= prot;
}

/**
 * Clear a bit or bits in the protect filed
 * @param obj pointer to an object
 * @param prot 'OR'-ed values from `lv_protect_t`
 */
void lv_obj_clear_protect(lv_obj_t * obj, uint8_t prot)
{
    prot = (~prot) & 0xFF;
    obj->protect &= prot;
}

/**
 * Set a an event handler function for an object.
 * Used by the user to react on event which happens with the object.
 * @param obj pointer to an object
 * @param cb the new event function
 */
void lv_obj_set_event_cb(lv_obj_t * obj, lv_event_cb_t cb)
{
    obj->event_cb = cb;
}

/**
 * Send an event to the object
 * @param obj pointer to an object
 * @param event the type of the event from `lv_event_t`.
 * @return LV_RES_OK: `obj` was not deleted in the event; LV_RES_INV: `obj` was deleted in the event
 */
lv_res_t lv_obj_send_event(lv_obj_t * obj, lv_event_t event)
{
    if(obj == NULL) return LV_RES_OK;

    /*If the event was send from an other event save the current states to restore it at the end*/
    lv_obj_t * prev_obj_act_event = obj_act_event;
    bool prev_obj_act_event_deleted = obj_act_event_deleted;

    obj_act_event = obj;
    obj_act_event_deleted = false;

    if(obj->event_cb) obj->event_cb(obj, event);

    bool deleted = obj_act_event_deleted;

    /*Restore the previous states*/
    obj_act_event = prev_obj_act_event;
    obj_act_event_deleted = prev_obj_act_event_deleted;

    if(deleted) {
        return LV_RES_INV;
    }

    if(obj->parent_event && obj->par) {
        lv_res_t res = lv_obj_send_event(obj->par, event);
        if(res != LV_RES_OK) return LV_RES_INV;
    }

    return LV_RES_OK;
}

/**
 * Set the a signal function of an object. Used internally by the library.
 * Always call the previous signal function in the new.
 * @param obj pointer to an object
 * @param cb the new signal function
 */
void lv_obj_set_signal_cb(lv_obj_t * obj, lv_signal_cb_t cb)
{
    obj->signal_cb = cb;
}

/**
 * Send an event to the object
 * @param obj pointer to an object
 * @param event the type of the event from `lv_event_t`.
 */
void lv_obj_send_signal(lv_obj_t * obj, lv_signal_t signal, void * param)
{
    if(obj->signal_cb) obj->signal_cb(obj, signal, param);
}

/**
 * Set a new design function for an object
 * @param obj pointer to an object
 * @param cb the new design function
 */
void lv_obj_set_design_cb(lv_obj_t * obj, lv_design_cb_t cb)
{
    obj->design_cb = cb;
}

/*----------------
 * Other set
 *--------------*/

/**
 * Allocate a new ext. data for an object
 * @param obj pointer to an object
 * @param ext_size the size of the new ext. data
 * @return Normal pointer to the allocated ext
 */
void * lv_obj_allocate_ext_attr(lv_obj_t * obj, uint16_t ext_size)
{
    obj->ext_attr = lv_mem_realloc(obj->ext_attr, ext_size);

    return (void *)obj->ext_attr;
}

/**
 * Send a 'LV_SIGNAL_REFR_EXT_SIZE' signal to the object
 * @param obj pointer to an object
 */
void lv_obj_refresh_ext_size(lv_obj_t * obj)
{
    obj->ext_size = 0;
    obj->signal_cb(obj, LV_SIGNAL_REFR_EXT_SIZE, NULL);

    lv_obj_invalidate(obj);
}

#if LV_USE_ANIMATION
/**
 * Animate an object
 * @param obj pointer to an object to animate
 * @param type type of animation from 'lv_anim_builtin_t'. 'OR' it with ANIM_IN or ANIM_OUT
 * @param time time of animation in milliseconds
 * @param delay delay before the animation in milliseconds
 * @param cb a function to call when the animation is ready
 */
void lv_obj_animate(lv_obj_t * obj, lv_anim_builtin_t type, uint16_t time, uint16_t delay, void (*cb)(lv_obj_t *))
{
    lv_obj_t * par = lv_obj_get_parent(obj);

    /*Get the direction*/
    bool out = (type & LV_ANIM_DIR_MASK) == LV_ANIM_IN ? false : true;
    type = type & (~LV_ANIM_DIR_MASK);

    lv_anim_t a;
    a.var = obj;
    a.time = time;
    a.act_time = (int32_t) - delay;
    a.end_cb = (void(*)(void *))cb;
    a.path = lv_anim_path_linear;
    a.playback_pause = 0;
    a.repeat_pause = 0;
    a.playback = 0;
    a.repeat = 0;

    /*Init to ANIM_IN*/
    switch(type) {
        case LV_ANIM_FLOAT_LEFT:
            a.fp = (void(*)(void *, int32_t))lv_obj_set_x;
            a.start = -lv_obj_get_width(obj);
            a.end = lv_obj_get_x(obj);
            break;
        case LV_ANIM_FLOAT_RIGHT:
            a.fp = (void(*)(void *, int32_t))lv_obj_set_x;
            a.start = lv_obj_get_width(par);
            a.end = lv_obj_get_x(obj);
            break;
        case LV_ANIM_FLOAT_TOP:
            a.fp = (void(*)(void *, int32_t))lv_obj_set_y;
            a.start = -lv_obj_get_height(obj);
            a.end = lv_obj_get_y(obj);
            break;
        case LV_ANIM_FLOAT_BOTTOM:
            a.fp = (void(*)(void *, int32_t))lv_obj_set_y;
            a.start = lv_obj_get_height(par);
            a.end = lv_obj_get_y(obj);
            break;
        case LV_ANIM_GROW_H:
            a.fp = (void(*)(void *, int32_t))lv_obj_set_width;
            a.start = 0;
            a.end = lv_obj_get_width(obj);
            break;
        case LV_ANIM_GROW_V:
            a.fp = (void(*)(void *, int32_t))lv_obj_set_height;
            a.start = 0;
            a.end = lv_obj_get_height(obj);
            break;
        case LV_ANIM_NONE:
            a.fp = NULL;
            a.start = 0;
            a.end = 0;
            break;
        default:
            break;
    }

    /*Swap start and end in case of ANIM OUT*/
    if(out != false) {
        int32_t tmp = a.start;
        a.start = a.end;
        a.end = tmp;
    }

    lv_anim_create(&a);
}

#endif

/*=======================
 * Getter functions
 *======================*/

/**
 * Return with the screen of an object
 * @param obj pointer to an object
 * @return pointer to a screen
 */
lv_obj_t * lv_obj_get_screen(const lv_obj_t * obj)
{
    const lv_obj_t * par = obj;
    const lv_obj_t * act_p;

    do {
        act_p = par;
        par = lv_obj_get_parent(act_p);
    } while(par != NULL);

    return (lv_obj_t *)act_p;
}

/**
 * Get the display of an object
 * @param scr pointer to an object
 * @return pointer the object's display
 */
lv_disp_t * lv_obj_get_disp(const lv_obj_t * obj)
{
    const lv_obj_t * scr;

    if(obj->par == NULL) scr = obj;         /*`obj` is a screen*/
    else scr = lv_obj_get_screen(obj);      /*get the screen of `obj`*/

    lv_disp_t * d;
    LV_LL_READ(LV_GC_ROOT(_lv_disp_ll), d) {
        lv_obj_t * s;
        LV_LL_READ(d->scr_ll, s) {
            if(s == scr) return d;
        }
    }

    LV_LOG_WARN("lv_scr_get_disp: screen not found")
    return NULL;
}

/*---------------------
 * Parent/children get
 *--------------------*/

/**
 * Returns with the parent of an object
 * @param obj pointer to an object
 * @return pointer to the parent of  'obj'
 */
lv_obj_t * lv_obj_get_parent(const lv_obj_t * obj)
{
    return obj->par;
}

/**
 * Iterate through the children of an object (start from the "youngest")
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
lv_obj_t * lv_obj_get_child(const lv_obj_t * obj, const lv_obj_t * child)
{
    lv_obj_t * result = NULL;

    if(child == NULL) {
        result = lv_ll_get_head(&obj->child_ll);
    } else {
        result = lv_ll_get_next(&obj->child_ll, child);
    }

    return result;
}

/**
 * Iterate through the children of an object (start from the "oldest")
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
lv_obj_t * lv_obj_get_child_back(const lv_obj_t * obj, const lv_obj_t * child)
{
    lv_obj_t * result = NULL;

    if(child == NULL) {
        result = lv_ll_get_tail(&obj->child_ll);
    } else {
        result = lv_ll_get_prev(&obj->child_ll, child);
    }

    return result;
}

/**
 * Count the children of an object (only children directly on 'obj')
 * @param obj pointer to an object
 * @return children number of 'obj'
 */
uint16_t lv_obj_count_children(const lv_obj_t * obj)
{
    lv_obj_t * i;
    uint16_t cnt = 0;

    LV_LL_READ(obj->child_ll, i) cnt++;

    return cnt;
}

/*---------------------
 * Coordinate get
 *--------------------*/

/**
 * Copy the coordinates of an object to an area
 * @param obj pointer to an object
 * @param cords_p pointer to an area to store the coordinates
 */
void lv_obj_get_coords(const lv_obj_t * obj, lv_area_t * cords_p)
{
    lv_area_copy(cords_p, &obj->coords);
}


/**
 * Get the x coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the left side of its parent
 */
lv_coord_t lv_obj_get_x(const lv_obj_t * obj)
{
    lv_coord_t rel_x;
    lv_obj_t * parent = lv_obj_get_parent(obj);
    rel_x = obj->coords.x1 - parent->coords.x1;

    return rel_x;
}

/**
 * Get the y coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the top of its parent
 */
lv_coord_t lv_obj_get_y(const lv_obj_t * obj)
{
    lv_coord_t rel_y;
    lv_obj_t * parent = lv_obj_get_parent(obj);
    rel_y = obj->coords.y1 - parent->coords.y1;

    return rel_y;
}

/**
 * Get the width of an object
 * @param obj pointer to an object
 * @return the width
 */
lv_coord_t lv_obj_get_width(const lv_obj_t * obj)
{
    return lv_area_get_width(&obj->coords);
}

/**
 * Get the height of an object
 * @param obj pointer to an object
 * @return the height
 */
lv_coord_t lv_obj_get_height(const lv_obj_t * obj)
{
    return lv_area_get_height(&obj->coords);
}

/**
 * Get that width reduced by the left and right padding.
 * @param obj pointer to an object
 * @return the width which still fits into the container
 */
lv_coord_t lv_obj_get_width_fit(lv_obj_t * obj)
{
    lv_style_t * style = lv_obj_get_style(obj);

    return lv_obj_get_width(obj) - style->body.padding.left - style->body.padding.right;
}

/**
 * Get that height reduced by the top an bottom padding.
 * @param obj pointer to an object
 * @return the height which still fits into the container
 */
lv_coord_t lv_obj_get_height_fit(lv_obj_t * obj)
{
    lv_style_t * style = lv_obj_get_style(obj);

    return lv_obj_get_width(obj) - style->body.padding.top - style->body.padding.bottom;
}
/**
 * Get the extended size attribute of an object
 * @param obj pointer to an object
 * @return the extended size attribute
 */
lv_coord_t lv_obj_get_ext_size(const lv_obj_t * obj)
{
    return obj->ext_size;
}

/**
 * Get the automatic realign property of the object.
 * @param obj pointer to an object
 * @return  true: auto realign is enabled; false: auto realign is disabled
 */
bool lv_obj_get_auto_realign(lv_obj_t * obj)
{
#if LV_OBJ_REALIGN
    return obj->realign.auto_realign ? true : false;
#else
    (void) obj;
    return false;
#endif
}

/*-----------------
 * Appearance get
 *---------------*/

/**
 * Get the style pointer of an object (if NULL get style of the parent)
 * @param obj pointer to an object
 * @return pointer to a style
 */
lv_style_t * lv_obj_get_style(const lv_obj_t * obj)
{
    lv_style_t * style_act = obj->style_p;
    if(style_act == NULL) {
        lv_obj_t * par = obj->par;

        while(par) {
            if(par->style_p) {
                if(par->style_p->glass == 0) {
#if LV_USE_GROUP == 0
                    style_act = par->style_p;
#else
                    /*Is a parent is focused then use then focused style*/
                    lv_group_t * g = lv_obj_get_group(par);
                    if(lv_group_get_focused(g) == par) {
                        style_act = lv_group_mod_style(g, par->style_p);
                    } else {
                        style_act = par->style_p;
                    }
#endif
                    break;
                }
            }
            par = par->par;
        }
    }
#if LV_USE_GROUP
    if(obj->group_p) {
        if(lv_group_get_focused(obj->group_p) == obj) {
            style_act = lv_group_mod_style(obj->group_p, style_act);
        }
    }
#endif

    if(style_act == NULL) style_act = &lv_style_plain;

    return style_act;
}

/*-----------------
 * Attribute get
 *----------------*/

/**
 * Get the hidden attribute of an object
 * @param obj pointer to an object
 * @return true: the object is hidden
 */
bool lv_obj_get_hidden(const lv_obj_t * obj)
{
    return obj->hidden == 0 ? false : true;
}

/**
 * Get the click enable attribute of an object
 * @param obj pointer to an object
 * @return true: the object is clickable
 */
bool lv_obj_get_click(const lv_obj_t * obj)
{
    return obj->click == 0 ? false : true;
}

/**
 * Get the top enable attribute of an object
 * @param obj pointer to an object
 * @return true: the auto top feture is enabled
 */
bool lv_obj_get_top(const lv_obj_t * obj)
{
    return obj->top == 0 ? false : true;
}

/**
 * Get the drag enable attribute of an object
 * @param obj pointer to an object
 * @return true: the object is dragable
 */
bool lv_obj_get_drag(const lv_obj_t * obj)
{
    return obj->drag == 0 ? false : true;
}

/**
 * Get the drag throw enable attribute of an object
 * @param obj pointer to an object
 * @return true: drag throw is enabled
 */
bool lv_obj_get_drag_throw(const lv_obj_t * obj)
{
    return obj->drag_throw == 0 ? false : true;
}

/**
 * Get the drag parent attribute of an object
 * @param obj pointer to an object
 * @return true: drag parent is enabled
 */
bool lv_obj_get_drag_parent(const lv_obj_t * obj)
{
    return obj->drag_parent == 0 ? false : true;
}

/**
 * Get the drag parent attribute of an object
 * @param obj pointer to an object
 * @return true: drag parent is enabled
 */
bool lv_obj_get_parent_event(const lv_obj_t * obj)
{
    return obj->parent_event == 0 ? false : true;
}

/**
 * Get the opa scale enable parameter
 * @param obj pointer to an object
 * @return true: opa scaling is enabled for this object and all children; false: no opa scaling
 */
lv_opa_t lv_obj_get_opa_scale_enable(const lv_obj_t * obj)
{
    return obj->opa_scale_en == 0 ? false : true;
}

/**
 * Get the opa scale parameter of an object
 * @param obj pointer to an object
 * @return opa scale [0..255]
 */
lv_opa_t lv_obj_get_opa_scale(const lv_obj_t * obj)
{
    const lv_obj_t * parent = obj;

    while(parent) {
        if(parent->opa_scale_en) return parent->opa_scale;
        parent = lv_obj_get_parent(parent);
    }

    return LV_OPA_COVER;
}

/**
 * Get the protect field of an object
 * @param obj pointer to an object
 * @return protect field ('OR'ed values of `lv_protect_t`)
 */
uint8_t lv_obj_get_protect(const lv_obj_t * obj)
{
    return obj->protect ;
}

/**
 * Check at least one bit of a given protect bitfield is set
 * @param obj pointer to an object
 * @param prot protect bits to test ('OR'ed values of `lv_protect_t`)
 * @return false: none of the given bits are set, true: at least one bit is set
 */
bool lv_obj_is_protected(const lv_obj_t * obj, uint8_t prot)
{
    return (obj->protect & prot) == 0 ? false : true ;
}

/**
 * Get the signal function of an object
 * @param obj pointer to an object
 * @return the signal function
 */
lv_signal_cb_t lv_obj_get_signal_func(const lv_obj_t * obj)
{
    return obj->signal_cb;
}

/**
 * Get the design function of an object
 * @param obj pointer to an object
 * @return the design function
 */
lv_design_cb_t lv_obj_get_design_func(const lv_obj_t * obj)
{
    return obj->design_cb;
}

/*------------------
 * Other get
 *-----------------*/

/**
 * Get the ext pointer
 * @param obj pointer to an object
 * @return the ext pointer but not the dynamic version
 *         Use it as ext->data1, and NOT da(ext)->data1
 */
void * lv_obj_get_ext_attr(const lv_obj_t * obj)
{
    return obj->ext_attr;
}

/**
 * Get object's and its ancestors type. Put their name in `type_buf` starting with the current type.
 * E.g. buf.type[0]="lv_btn", buf.type[1]="lv_cont", buf.type[2]="lv_obj"
 * @param obj pointer to an object which type should be get
 * @param buf pointer to an `lv_obj_type_t` buffer to store the types
 */
void lv_obj_get_type(lv_obj_t * obj, lv_obj_type_t * buf)
{
    lv_obj_type_t tmp;

    memset(buf, 0, sizeof(lv_obj_type_t));
    memset(&tmp, 0, sizeof(lv_obj_type_t));

    obj->signal_cb(obj, LV_SIGNAL_GET_TYPE, &tmp);

    uint8_t cnt;
    for(cnt = 0; cnt < LV_MAX_ANCESTOR_NUM; cnt++) {
        if(tmp.type[cnt] == NULL) break;
    }


    /*Swap the order. The real type comes first*/
    uint8_t i;
    for(i = 0; i < cnt; i++) {
        buf->type[i] = tmp.type[cnt - 1 - i];
    }
}

#if LV_USE_USER_DATA_SINGLE
/**
 * Get a pointer to the object's user data
 * @param obj pointer to an object
 * @return pointer to the user data
 */
lv_obj_user_data_t * lv_obj_get_user_data(lv_obj_t * obj)
{
    return &obj->user_data;
}
#endif

#if LV_USE_GROUP
/**
 * Get the group of the object
 * @param obj pointer to an object
 * @return the pointer to group of the object
 */
void * lv_obj_get_group(const lv_obj_t * obj)
{
    return obj->group_p;
}

/**
 * Tell whether the ohe object is the focused object of a group or not.
 * @param obj pointer to an object
 * @return true: the object is focused, false: the object is not focused or not in a group
 */
bool lv_obj_is_focused(const lv_obj_t * obj)
{
    if(obj->group_p) {
        if(lv_group_get_focused(obj->group_p) == obj) return true;
    }

    return false;
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the base objects.
 * @param obj pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode LV_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             LV_DESIGN_DRAW: draw the object (always return 'true')
 * @param return true/false, depends on 'mode'
 */
static bool lv_obj_design(lv_obj_t * obj, const  lv_area_t * mask_p, lv_design_mode_t mode)
{
    if(mode == LV_DESIGN_COVER_CHK) {

        /*Most trivial test. The mask is fully  IN the object? If no it surely not covers it*/
        if(lv_area_is_in(mask_p, &obj->coords) == false) return false;

        /*Can cover the area only if fully solid (no opacity)*/
        lv_style_t * style = lv_obj_get_style(obj);
        if(style->body.opa != LV_OPA_COVER) return false;

        /* Because of the radius it is not sure the area is covered
         * Check the areas where there is no radius*/
        uint16_t r = style->body.radius;

        if(r == LV_RADIUS_CIRCLE) return false;

        lv_area_t area_tmp;

        /*Check horizontally without radius*/
        lv_obj_get_coords(obj, &area_tmp);
        area_tmp.x1 += r;
        area_tmp.x2 -= r;
        if(lv_area_is_in(mask_p, &area_tmp) == false) return false;

        /*Check vertically without radius*/
        lv_obj_get_coords(obj, &area_tmp);
        area_tmp.y1 += r;
        area_tmp.y2 -= r;
        if(lv_area_is_in(mask_p, &area_tmp) == false) return false;

    } else if(mode == LV_DESIGN_DRAW_MAIN) {
        lv_style_t * style = lv_obj_get_style(obj);
        lv_draw_rect(&obj->coords, mask_p, style, lv_obj_get_opa_scale(obj));
    }

    return true;
}

/**
 * Signal function of the basic object
 * @param obj pointer to an object
 * @param sign signal type
 * @param param parameter for the signal (depends on signal type)
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_obj_signal(lv_obj_t * obj, lv_signal_t sign, void * param)
{
    (void)param;

    lv_res_t res = LV_RES_OK;

    lv_style_t * style = lv_obj_get_style(obj);

    lv_indev_t *indev_act = lv_indev_get_act();

    if(sign > _LV_SIGNAL_FEEDBACK_SECTION_START && sign < _LV_SIGNAL_FEEDBACK_SECTION_END) {
		if(indev_act != NULL && indev_act->feedback != NULL)
			indev_act->feedback(indev_act, sign);
    }

    if(sign == LV_SIGNAL_CHILD_CHG) {
        /*Return 'invalid' if the child change signal is not enabled*/
        if(lv_obj_is_protected(obj, LV_PROTECT_CHILD_CHG) != false) res = LV_RES_INV;
    } else if(sign == LV_SIGNAL_REFR_EXT_SIZE) {
        if(style->body.shadow.width > obj->ext_size) obj->ext_size = style->body.shadow.width;
    } else if(sign ==  LV_SIGNAL_STYLE_CHG) {
        lv_obj_refresh_ext_size(obj);
    } else if(sign ==  LV_SIGNAL_GET_TYPE) {
        lv_obj_type_t * buf = param;
        buf->type[0] = "lv_obj";
    }

    return res;
}

/**
 * Reposition the children of an object. (Called recursively)
 * @param obj pointer to an object which children will be repositioned
 * @param x_diff x coordinate shift
 * @param y_diff y coordinate shift
 */
static void refresh_children_position(lv_obj_t * obj, lv_coord_t x_diff, lv_coord_t y_diff)
{
    lv_obj_t * i;
    LV_LL_READ(obj->child_ll, i) {
        i->coords.x1 += x_diff;
        i->coords.y1 += y_diff;
        i->coords.x2 += x_diff;
        i->coords.y2 += y_diff;

        refresh_children_position(i, x_diff, y_diff);
    }
}

/**
 * Refresh the style of all children of an object. (Called recursively)
 * @param style_p refresh objects only with this style.
 * @param obj pointer to an object
 */
static void report_style_mod_core(void * style_p, lv_obj_t * obj)
{
    lv_obj_t * i;
    LV_LL_READ(obj->child_ll, i) {
        if(i->style_p == style_p || style_p == NULL) {
            refresh_children_style(i);
            lv_obj_refresh_style(i);
        }

        report_style_mod_core(style_p, i);
    }
}

/**
 * Recursively refresh the style of the children. Go deeper until a not NULL style is found
 * because the NULL styles are inherited from the parent
 * @param obj pointer to an object
 */
static void refresh_children_style(lv_obj_t * obj)
{
    lv_obj_t * child = lv_obj_get_child(obj, NULL);
    while(child != NULL) {
        if(child->style_p == NULL) {
            refresh_children_style(child);     /*Check children too*/
            lv_obj_refresh_style(child);       /*Notify the child about the style change*/
        } else if(child->style_p->glass) {
            /*Children with 'glass' parent might be effected if their style == NULL*/
            refresh_children_style(child);
        }
        child = lv_obj_get_child(obj, child);
    }
}

/**
 * Called by 'lv_obj_del' to delete the children objects
 * @param obj pointer to an object (all of its children will be deleted)
 */
static void delete_children(lv_obj_t * obj)
{

    if(obj_act_event == obj && obj_act_event_deleted == false) obj_act_event_deleted = true;

    lv_obj_t * i;
    lv_obj_t * i_next;
    i = lv_ll_get_head(&(obj->child_ll));

    /*Remove from the group; remove before transversing children so that 
     * the object still has access to all children during the 
     * LV_SIGNAL_DEFOCUS call*/
#if LV_USE_GROUP
    bool was_focused = false;

    if(obj->group_p) {
        if(lv_group_get_focused(obj->group_p) == obj) was_focused = true;
        lv_group_remove_obj(obj);
    }
#endif

    while(i != NULL) {
        /*Get the next object before delete this*/
        i_next = lv_ll_get_next(&(obj->child_ll), i);

        /*Call the recursive del to the child too*/
        delete_children(i);

        /*Set i to the next node*/
        i = i_next;
    }

    /*Remove the animations from this object*/
#if LV_USE_ANIMATION
    lv_anim_del(obj, NULL);
#endif

    /* Reset the input devices if
     * the object to delete is used*/
    lv_indev_t * indev = lv_indev_next(NULL);
    while(indev) {
        if(indev->proc.types.pointer.act_obj == obj || indev->proc.types.pointer.last_obj == obj) {
            lv_indev_reset(indev);
        }
#if LV_USE_GROUP
        if(was_focused) {
            lv_indev_reset(indev);
        }
#endif
        indev = lv_indev_next(indev);
    }

    /*Remove the object from parent's children list*/
    lv_obj_t * par = lv_obj_get_parent(obj);
    lv_ll_rem(&(par->child_ll), obj);

    /* Clean up the object specific data*/
    obj->signal_cb(obj, LV_SIGNAL_CLEANUP, NULL);

    /*Delete the base objects*/
    if(obj->ext_attr != NULL)  lv_mem_free(obj->ext_attr);
    lv_mem_free(obj); /*Free the object itself*/

}