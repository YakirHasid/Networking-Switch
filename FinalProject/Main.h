#ifndef MAIN_H
#define MAIN_H
#define FORMAT_WORD_LEN 6
#define FILE_PATH_INDEX 0
#define FILE_ROUTE_INDEX 1
#define FILE_PORT_IN_INDEX_START 2
#define FILE_PORT1_IN_INDEX 2
#define FILE_PORT2_IN_INDEX 3
#define FILE_PORT3_IN_INDEX 4
#define FILE_PORT4_IN_INDEX 5

#define OUTPUT_PORT_1 1
#define OUTPUT_PORT_2 2
#define OUTPUT_PORT_3 3
#define OUTPUT_PORT_4 4

#define NUM_OF_PORTS 4

#define PORT_INIT_VALUE -1

#include "Packet.h"

typedef struct switchManager {
	FILE * pFileRoute;
	FILE * pFilePortIn[NUM_OF_PORTS];
	FILE * pFilePortOut[NUM_OF_PORTS];

	S_node * routeTable;

	packet * portPacket[NUM_OF_PORTS];

	S_Out_Qs_mgr * qsMgr[NUM_OF_PORTS];	
}switchManager;

switchManager * switchInit(int argc, char * argv[]);
void readPacketFromPorts(switchManager * swMng);
int smallestTimePacket(const switchManager * swMng, packet ** smallest);
void advancePortPacket(switchManager * swMng, int portToAdance);
packet * dequeueFromMgr(S_Out_Qs_mgr * qMgr);
void sendPacket(switchManager * const swMng, packet * pktToSend);
FILE * portToSend(const switchManager * const swMng, S_node * node);
Bool isMoreToQueue(const switchManager * const swMng);
void closeAllFiles(switchManager * swMng);
void freeAllSwMng(switchManager * swMng);
void freePackets(switchManager * swMng);
void freePacket(packet * pktToRem);
void freeQueueMgr(switchManager * swMng);
void freeQueues(S_Out_Qs_mgr * qsMgr);
void enqueueAllPackets(switchManager * swMng);
void dequeueAllPackets(switchManager * swMng);

#endif