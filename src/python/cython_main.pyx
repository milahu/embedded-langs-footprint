# tag::declaration[]
# transpiled into #include directive
cdef extern from "proc.hh":

     # used as type declarations for the proper inference
     # not necessary the same as original C declarations
     size_t read_rss()

# `cpdef` means that the code could use C functions, and available in Python
# `public` makes the declaration included into the generated header
cpdef public cy_read():
    rss = read_rss()
    return "world"
# end::declaration[]
