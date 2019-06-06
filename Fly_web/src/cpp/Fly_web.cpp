#include "Fly_web.h"
#include "Fly_string.h"

std::string Fly_web::RAW::IPPORT::getIpPort()
{ 
	return Fly_string::format("%s:%d", ip.c_str(), port); 
}
