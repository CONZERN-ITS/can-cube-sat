# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS
	"hello_world_main.c"
	
	"router/rt_sender_ctx.h"
	"router/rt_endpoint_iface.h"
	"router/router.h"
	"router/router.c"
	
	"component_one/comp1.h"
	"component_one/comp1.c"

	"component_two/comp2.h"
	"component_two/comp2.c"

)

set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
