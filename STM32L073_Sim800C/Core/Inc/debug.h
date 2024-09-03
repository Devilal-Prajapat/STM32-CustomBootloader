/*
 * debug.h
 *
 *  Created on: Aug 8, 2024
 *      Author: devilalprajapat
 */

#ifndef DEBUG_H_
#define DEBUG_H_

void send_log(char *ptr);

#define DEBUG_LOG(tag, msg)  { \
								send_log("\r\n[ ");\
								send_log((char *)tag);\
								send_log(" ] ");\
								send_log((char *)msg);\
							 }
#endif /* DEBUG_H_ */
