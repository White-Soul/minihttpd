set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

add_library(utils STATIC
    Interface.hpp
    mysqlconn.hpp
    Request.hpp
    Response.hpp
    Result.hpp
    Router.hpp
    Server.hpp
    Servlet.hpp
    Session.hpp
    utils.hpp
)