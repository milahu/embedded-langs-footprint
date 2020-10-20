cdef extern from "proc.hh":
     size_t read_rss()

# tag::declaration[]
cpdef public cy_read():
    rss = read_rss()
    return "world"
# end::declaration[]
