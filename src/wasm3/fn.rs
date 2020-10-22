#![no_std]

extern {
    fn new_block() -> i32;
    fn append_to_block(id: i32, v: i32) -> i32;
    fn join(dest: i32, src: i32) -> i32;
    fn read() -> i32;
}

fn m3_str(s: &str) -> i32 {
    let id = unsafe { new_block() };
    for b in s.as_bytes() {
        unsafe { append_to_block(id, (*b).into()) };
    }
    id
}

// tag::src[]
#[no_mangle]
pub fn r#fn() -> i32 {
    return unsafe { join(m3_str("Hello, "), read()) };
}
// end::src[]

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_panic: &PanicInfo<'_>) -> ! {
    loop {}
}
