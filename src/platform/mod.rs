//! Platform abstraction layer for window and display operations.

#[cfg(target_os = "linux")]
mod linux;

#[cfg(target_os = "windows")]
mod windows;

/// Safely toggles fullscreen mode across supported platforms.
///
/// - On Linux X11, dispatches EWMH client messages and restores keyboard focus,
///   working around Miniquad's upstream `XUnmapWindow` focus-loss bug.
/// - On Windows, manages window style transitions and restores standard 960x720 window
///   dimensions centered on screen when returning to windowed mode.
/// - On other platforms, falls back cleanly to Macroquad's native implementation.
pub fn toggle_fullscreen(is_fullscreen: &mut bool) {
    #[cfg(target_os = "linux")]
    {
        *is_fullscreen = !*is_fullscreen;
        if unsafe { linux::toggle_fullscreen_x11() } {
            return;
        }
        macroquad::window::set_fullscreen(*is_fullscreen);
    }

    #[cfg(target_os = "windows")]
    {
        windows::toggle_fullscreen_windows(is_fullscreen);
    }

    #[cfg(not(any(target_os = "linux", target_os = "windows")))]
    {
        *is_fullscreen = !*is_fullscreen;
        macroquad::window::set_fullscreen(*is_fullscreen);
    }
}

/// Computes the inverted state for fullscreen toggling.
pub fn toggle_fullscreen_state(current: bool) -> bool {
    !current
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_toggle_fullscreen_state_inversion() {
        assert!(toggle_fullscreen_state(false));
        assert!(!toggle_fullscreen_state(true));
    }
}
