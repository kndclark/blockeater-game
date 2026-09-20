//! Windows platform-specific window and display operations.

#[cfg(target_os = "windows")]
pub fn toggle_fullscreen_windows(is_fullscreen: &mut bool) {
    *is_fullscreen = !*is_fullscreen;
    if *is_fullscreen {
        macroquad::window::set_fullscreen(true);
    } else {
        macroquad::window::set_fullscreen(false);
        // Miniquad on Windows retains the fullscreen display resolution when exiting fullscreen.
        // Explicitly request restoring standard 960x720 window dimensions.
        macroquad::window::request_new_screen_size(960.0, 720.0);

        // Center the window on the desktop via Win32 if the window handle is found
        unsafe {
            center_window();
        }
    }
}

#[cfg(target_os = "windows")]
unsafe fn center_window() {
    #[link(name = "user32")]
    extern "system" {
        fn FindWindowA(
            lpClassName: *const libc::c_char,
            lpWindowName: *const libc::c_char,
        ) -> *mut libc::c_void;
        fn SetWindowPos(
            hWnd: *mut libc::c_void,
            hWndInsertAfter: *mut libc::c_void,
            X: libc::c_int,
            Y: libc::c_int,
            cx: libc::c_int,
            cy: libc::c_int,
            uFlags: libc::c_uint,
        ) -> libc::c_int;
        fn GetSystemMetrics(nIndex: libc::c_int) -> libc::c_int;
    }

    let class_name = c"MINIQUADAPP";
    let window_name = c"THE BLOCKEATER";
    let hwnd = FindWindowA(class_name.as_ptr(), window_name.as_ptr());
    if !hwnd.is_null() {
        let screen_w = GetSystemMetrics(0); // SM_CXSCREEN
        let screen_h = GetSystemMetrics(1); // SM_CYSCREEN
        let x = (screen_w - 960).max(0) / 2;
        let y = (screen_h - 720).max(0) / 2;
        // SWP_FRAMECHANGED (0x0020) | SWP_NOZORDER (0x0004)
        SetWindowPos(hwnd, std::ptr::null_mut(), x, y, 960, 720, 0x0024);
    }
}
