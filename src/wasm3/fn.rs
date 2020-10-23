use std::convert::TryInto;

extern {
    fn block_new() -> i32;
    fn block_pushback(id: i32, v: i32) -> i32;
    fn block_size(id: i32) -> i32;
    fn block_read(id: i32, idx: i32) -> i32;

    fn read() -> i32;
}

fn store_m3(s: &str) -> i32 {
    let id = unsafe { block_new() };
    for b in s.as_bytes() {
        let byte = (*b).into();
        unsafe { block_pushback(id, byte) };
    }
    id
}

fn load_m3(id: i32) -> String {
    let sz = unsafe { block_size(id) };
    let mut s = String::with_capacity(sz.try_into().unwrap());
    for i in 0..sz {
        let byte = unsafe { block_read(id, i) as u8 as char };
        s.push(byte)
    }
    s
}

// tag::src[]
#[no_mangle]
pub fn r#fn() -> i32 {
    let read_str = load_m3(unsafe { read() });
    return store_m3(&format!("Hello, {}", read_str));
}
// end::src[]
