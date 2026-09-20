//! Linux X11 dynamic FFI implementation.
//!
//! Provides direct EWMH client message dispatch and input focus restoration
//! without requiring static X11 link-time dependencies or triggering Miniquad's
//! unmap focus loss bug.

#[cfg(target_os = "linux")]
pub unsafe fn toggle_fullscreen_x11() -> bool {
    let lib = libc::dlopen(c"libX11.so.6".as_ptr(), libc::RTLD_LAZY);
    if lib.is_null() {
        return false;
    }

    type XOpenDisplayFn = unsafe extern "C" fn(*const libc::c_char) -> *mut libc::c_void;
    type XDefaultRootWindowFn = unsafe extern "C" fn(*mut libc::c_void) -> libc::c_ulong;
    type XInternAtomFn =
        unsafe extern "C" fn(*mut libc::c_void, *const libc::c_char, libc::c_int) -> libc::c_ulong;
    type XFetchNameFn = unsafe extern "C" fn(
        *mut libc::c_void,
        libc::c_ulong,
        *mut *mut libc::c_char,
    ) -> libc::c_int;
    type XQueryTreeFn = unsafe extern "C" fn(
        *mut libc::c_void,
        libc::c_ulong,
        *mut libc::c_ulong,
        *mut libc::c_ulong,
        *mut *mut libc::c_ulong,
        *mut libc::c_uint,
    ) -> libc::c_int;
    type XGetWindowPropertyFn = unsafe extern "C" fn(
        *mut libc::c_void,
        libc::c_ulong,
        libc::c_ulong,
        libc::c_long,
        libc::c_long,
        libc::c_int,
        libc::c_ulong,
        *mut libc::c_ulong,
        *mut libc::c_int,
        *mut libc::c_ulong,
        *mut libc::c_ulong,
        *mut *mut libc::c_uchar,
    ) -> libc::c_int;
    type XSendEventFn = unsafe extern "C" fn(
        *mut libc::c_void,
        libc::c_ulong,
        libc::c_int,
        libc::c_long,
        *mut libc::c_void,
    ) -> libc::c_int;
    type XSetInputFocusFn = unsafe extern "C" fn(
        *mut libc::c_void,
        libc::c_ulong,
        libc::c_int,
        libc::c_ulong,
    ) -> libc::c_int;
    type XFlushFn = unsafe extern "C" fn(*mut libc::c_void) -> libc::c_int;
    type XFreeFn = unsafe extern "C" fn(*mut libc::c_void) -> libc::c_int;
    type XCloseDisplayFn = unsafe extern "C" fn(*mut libc::c_void) -> libc::c_int;

    let x_open_display: XOpenDisplayFn =
        std::mem::transmute(libc::dlsym(lib, c"XOpenDisplay".as_ptr()));
    let x_default_root: XDefaultRootWindowFn =
        std::mem::transmute(libc::dlsym(lib, c"XDefaultRootWindow".as_ptr()));
    let x_intern_atom: XInternAtomFn =
        std::mem::transmute(libc::dlsym(lib, c"XInternAtom".as_ptr()));
    let x_fetch_name: XFetchNameFn = std::mem::transmute(libc::dlsym(lib, c"XFetchName".as_ptr()));
    let x_query_tree: XQueryTreeFn = std::mem::transmute(libc::dlsym(lib, c"XQueryTree".as_ptr()));
    let x_get_window_property: XGetWindowPropertyFn =
        std::mem::transmute(libc::dlsym(lib, c"XGetWindowProperty".as_ptr()));
    let x_send_event: XSendEventFn = std::mem::transmute(libc::dlsym(lib, c"XSendEvent".as_ptr()));
    let x_set_input_focus: XSetInputFocusFn =
        std::mem::transmute(libc::dlsym(lib, c"XSetInputFocus".as_ptr()));
    let x_flush: XFlushFn = std::mem::transmute(libc::dlsym(lib, c"XFlush".as_ptr()));
    let x_free: XFreeFn = std::mem::transmute(libc::dlsym(lib, c"XFree".as_ptr()));
    let x_close_display: XCloseDisplayFn =
        std::mem::transmute(libc::dlsym(lib, c"XCloseDisplay".as_ptr()));

    let display = x_open_display(std::ptr::null());
    if display.is_null() {
        libc::dlclose(lib);
        return false;
    }

    let root = x_default_root(display);
    let wm_state = x_intern_atom(display, c"_NET_WM_STATE".as_ptr(), 0);
    let wm_fullscreen = x_intern_atom(display, c"_NET_WM_STATE_FULLSCREEN".as_ptr(), 0);
    let net_client_list = x_intern_atom(display, c"_NET_CLIENT_LIST".as_ptr(), 0);
    let icccm_wm_state = x_intern_atom(display, c"WM_STATE".as_ptr(), 0);

    let mut target_window: Option<libc::c_ulong> = None;

    // 1. Primary EWMH discovery: inspect _NET_CLIENT_LIST on the root window.
    // This list is maintained directly by the Window Manager and contains ONLY
    // actual client windows, explicitly excluding WM decoration frame windows.
    let xa_window: libc::c_ulong = 33;
    let mut actual_type: libc::c_ulong = 0;
    let mut actual_format: libc::c_int = 0;
    let mut nitems: libc::c_ulong = 0;
    let mut bytes_after: libc::c_ulong = 0;
    let mut prop_return: *mut libc::c_uchar = std::ptr::null_mut();

    let prop_status = x_get_window_property(
        display,
        root,
        net_client_list,
        0,
        1024,
        0,
        xa_window,
        &mut actual_type,
        &mut actual_format,
        &mut nitems,
        &mut bytes_after,
        &mut prop_return,
    );

    if prop_status == 0 && !prop_return.is_null() && nitems > 0 {
        let windows =
            std::slice::from_raw_parts(prop_return as *const libc::c_ulong, nitems as usize);
        for &w in windows {
            let mut name_ptr: *mut libc::c_char = std::ptr::null_mut();
            if x_fetch_name(display, w, &mut name_ptr) != 0 && !name_ptr.is_null() {
                let cstr = std::ffi::CStr::from_ptr(name_ptr);
                let matches = cstr.to_string_lossy().contains("THE BLOCKEATER");
                x_free(name_ptr as *mut _);
                if matches {
                    target_window = Some(w);
                    break;
                }
            }
        }
        x_free(prop_return as *mut _);
    }

    // 2. Fallback discovery for non-EWMH window managers:
    // Traverses the window tree post-order (children first) and requires WM_STATE
    // to ensure we target the client window rather than a WM decoration frame.
    if target_window.is_none() {
        unsafe fn find_client_tree(
            display: *mut libc::c_void,
            current: libc::c_ulong,
            icccm_wm_state: libc::c_ulong,
            fetch_name: XFetchNameFn,
            query_tree: XQueryTreeFn,
            get_prop: XGetWindowPropertyFn,
            free_fn: XFreeFn,
        ) -> Option<libc::c_ulong> {
            let mut root_ret: libc::c_ulong = 0;
            let mut parent_ret: libc::c_ulong = 0;
            let mut children_ret: *mut libc::c_ulong = std::ptr::null_mut();
            let mut nchildren: libc::c_uint = 0;

            if query_tree(
                display,
                current,
                &mut root_ret,
                &mut parent_ret,
                &mut children_ret,
                &mut nchildren,
            ) != 0
                && !children_ret.is_null()
            {
                let slice = std::slice::from_raw_parts(children_ret, nchildren as usize);
                let mut found = None;
                for &child in slice {
                    if let Some(w) = find_client_tree(
                        display,
                        child,
                        icccm_wm_state,
                        fetch_name,
                        query_tree,
                        get_prop,
                        free_fn,
                    ) {
                        found = Some(w);
                        break;
                    }
                }
                free_fn(children_ret as *mut _);
                if found.is_some() {
                    return found;
                }
            }

            let mut name_ptr: *mut libc::c_char = std::ptr::null_mut();
            if fetch_name(display, current, &mut name_ptr) != 0 && !name_ptr.is_null() {
                let cstr = std::ffi::CStr::from_ptr(name_ptr);
                let matches = cstr.to_string_lossy().contains("THE BLOCKEATER");
                free_fn(name_ptr as *mut _);
                if matches {
                    // Verify that this window has WM_STATE (client window indicator)
                    let mut actual_type: libc::c_ulong = 0;
                    let mut actual_format: libc::c_int = 0;
                    let mut nitems: libc::c_ulong = 0;
                    let mut bytes_after: libc::c_ulong = 0;
                    let mut prop: *mut libc::c_uchar = std::ptr::null_mut();
                    let st = get_prop(
                        display,
                        current,
                        icccm_wm_state,
                        0,
                        0,
                        0,
                        0,
                        &mut actual_type,
                        &mut actual_format,
                        &mut nitems,
                        &mut bytes_after,
                        &mut prop,
                    );
                    if st == 0 && actual_type != 0 {
                        if !prop.is_null() {
                            free_fn(prop as *mut _);
                        }
                        return Some(current);
                    }
                }
            }

            None
        }

        target_window = find_client_tree(
            display,
            root,
            icccm_wm_state,
            x_fetch_name,
            x_query_tree,
            x_get_window_property,
            x_free,
        );
    }

    if let Some(window) = target_window {
        #[repr(C)]
        struct XClientMessageEvent {
            type_: libc::c_int,
            serial: libc::c_ulong,
            send_event: libc::c_int,
            display: *mut libc::c_void,
            window: libc::c_ulong,
            message_type: libc::c_ulong,
            format: libc::c_int,
            data: [libc::c_long; 5],
        }

        let mut ev = XClientMessageEvent {
            type_: 33, // ClientMessage
            serial: 0,
            send_event: 1,
            display,
            window,
            message_type: wm_state,
            format: 32,
            data: [
                2, // _NET_WM_STATE_TOGGLE
                wm_fullscreen as libc::c_long,
                0,
                1, // source indication: normal application
                0,
            ],
        };

        let mask = (1 << 20) | (1 << 17); // SubstructureRedirectMask | SubstructureNotifyMask
        x_send_event(
            display,
            root,
            0,
            mask,
            &mut ev as *mut _ as *mut libc::c_void,
        );
        x_set_input_focus(
            display, window, 2, /* RevertToParent */
            0, /* CurrentTime */
        );
        x_flush(display);
    }

    x_close_display(display);
    libc::dlclose(lib);

    target_window.is_some()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_linux_x11_dynamic_loader_clean_exit() {
        // In headless testing or environments without X11 server,
        // toggle_fullscreen_x11 should safely handle missing Display and return false without panicking.
        let result = unsafe { toggle_fullscreen_x11() };
        // If display is not running or no Blockeater window exists, result is false;
        // if running under X11 with window, it returns a valid boolean.
        let _ = result;
    }
}
