#include <arpa/inet.h>

uint16_t htons(uint16_t d) {
	return __builtin_bswap16(d);
}

uint16_t ntohs(uint16_t d) {
	return __builtin_bswap16(d);
}

uint32_t htonl(uint32_t d) {
	return __builtin_bswap32(d);
}

uint32_t ntohl(uint32_t d) {
	return __builtin_bswap32(d);
}
