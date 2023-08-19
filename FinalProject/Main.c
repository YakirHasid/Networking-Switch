#define _CRT_SECURE_NO_WARNINGS
#include "Packet.h"
#include "Main.h"
#include <stdio.h>
#include <stdlib.h>

void main(int argc, char * argv[])
{	
	// first condition check to see if the program can run, number of files needs to be as defined in FORMAT_WORD_LEN
	if (argc != FORMAT_WORD_LEN)
	{
		printf("[ERROR] FORMAT INVALID, PLEASE ENTER RUN FILE AND 5 FILE NAMES");
		return;
	}

	// second condition check to see if the program can run, the number of ports needs to be 4 for this implementation
	if (NUM_OF_PORTS != 4)
	{
		printf("Current Implementation supports only a fixed number of ports which is 4 port in and 4 port out.\n");
		return;
	}

	// init the switch manager
	switchManager * swMng = switchInit(argc, argv);

	// build the route table
	swMng->routeTable = build_route_table(swMng->pFileRoute, swMng->routeTable);

	// print the route table
	print_routing_table(swMng->routeTable);

	// queues all packets from the .in files
	enqueueAllPackets(swMng);

	// dequeues all packets and prints them to the .out files
	dequeueAllPackets(swMng);

	// closes all the files pointers inside the switch manager
	closeAllFiles(swMng);	

	// frees all the mallocs initiated inside the switch manager
	freeAllSwMng(swMng);	

	printf("[SYSTEM MESSAGE] DONE WRITING TO OUTPUT PORT FILES!\n");

	system("pause");	
}

/*
Function name: switchInit
Description : Initializes the switch manager.
Input: Integer argc, Char argv
Output: Pointer to a switchManager
Algorithm: Allocates memory for switchManager, opens all the files in  portIn and portOut, mallocs the port packets..
		   .. sets the routing table to NULL and returns the switchManager pointer.
*/
switchManager * switchInit(int argc, char * argv[])
{
	// allocate memory for the switch manager
	switchManager * swMng = (switchManager *)(malloc(sizeof(switchManager)));
	if (!swMng)
	{
		printf("[ERROR] ERROR MALLOC SWITCH MANAGER.");
		exit(1);
	}

	// open the route file
	swMng->pFileRoute = fopen(argv[FILE_ROUTE_INDEX], "rt");
	if (!swMng->pFileRoute)
	{
		printf("[ERROR] ERROR OPENNING ROUTE FILE.");
		exit(1);
	}

	// open all the port in files
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		// open "portX.in" -> "portX" + ".in"
		swMng->pFilePortIn[i] = fopen(argv[FILE_PORT_IN_INDEX_START + i], "rt");

		if (!swMng->pFilePortIn[i])
		{
			printf("[ERROR] ERROR OPENNING PORT IN FILE.");
			exit(1);
		}

	}

	// open all the port out files
	swMng->pFilePortOut[0] = fopen("port1.out", "wt");
	if (!swMng->pFilePortOut[0])
	{
		printf("[ERROR] ERROR OPENNING PORT OUT FILE.");
		exit(1);
	}

	swMng->pFilePortOut[1] = fopen("port2.out", "wt");
	if (!swMng->pFilePortOut[1])
	{
		printf("[ERROR] ERROR OPENNING PORT OUT FILE.");
		exit(1);
	}

	swMng->pFilePortOut[2] = fopen("port3.out", "wt");
	if (!swMng->pFilePortOut[2])
	{
		printf("[ERROR] ERROR OPENNING PORT OUT FILE.");
		exit(1);
	}

	swMng->pFilePortOut[3] = fopen("port4.out", "wt");
	if (!swMng->pFilePortOut[3])
	{
		printf("[ERROR] ERROR OPENNING PORT OUT FILE.");
		exit(1);
	}

	// allocate memory for all the port packets
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		swMng->portPacket[i] = (packet *)malloc(sizeof(packet));

		if (!swMng->portPacket[i])
		{
			printf("[ERROR] ERROR MALLOC PORT PACKET.");
			exit(1);
		}
	}

	// allocate memory for all the queue managers and for all the queues inside of them.
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		swMng->qsMgr[i] = (S_Out_Qs_mgr *)malloc(sizeof(S_Out_Qs_mgr));
		if (!swMng->qsMgr[i])
		{
			printf("[ERROR] ERROR MALLOC PORT PACKET.");
			exit(1);
		}

		swMng->qsMgr[i]->p0 = (Queue *)malloc(sizeof(Queue));
		swMng->qsMgr[i]->p0->head = NULL;
		swMng->qsMgr[i]->p0->tail = NULL;		

		swMng->qsMgr[i]->p1 = (Queue *)malloc(sizeof(Queue));
		swMng->qsMgr[i]->p1->head = NULL;
		swMng->qsMgr[i]->p1->tail = NULL;
	}

	// read the first packets to the port packet
	readPacketFromPorts(swMng);

	// set the route table to be NULL in initialization
	swMng->routeTable = NULL;

	return swMng;
}

/*
Function name: readPacketFromPorts
Description : Initializes the switch manager.
Input: Pointer to a switchManager swMng
Output: none
Algorithm: Reads a packet for each port in available, puts the packet inside the matching port packet.
*/
void readPacketFromPorts(switchManager * swMng)
{
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		packet_read(swMng->pFilePortIn[i], swMng->portPacket[i]);

		// if the file has reached to an end, it means the read packet is 'garbage' and needs to be freed
		if (feof(swMng->pFilePortIn[i]))
		{
			free(swMng->portPacket[i]);
			swMng->portPacket[i] = NULL;
		}
	}
}

/*
Function name: smallestTimePacket
Description : Initializes the switch manager.
Input: Pointer to a Constant switchManager swMng, Pointer to a Pointer to a packet smallest
Output: Integer
Algorithm: For each port in port packet, check if it's smaller than smallest, if it is - set smallest to point at it..
		   Also, return the port of the smallest.
*/
int smallestTimePacket(const switchManager * swMng, packet ** smallest)
{
	// init the port of smallest with a unique value
	int portOfSmallest = PORT_INIT_VALUE;
	int i = 0;

	// check if the pointer to the pointer of smallest is valid
	if (!smallest)
	{
		printf("[ERROR] Pointer to the pointer of Smallest is NULL");
		return PORT_INIT_VALUE;
	}		

	for (; i < NUM_OF_PORTS; i++)
	{
		// if a smallest is NULL AND the port packet is not NULL, set the smallest as the port and port of smallest to be the index of it.
		if (*(smallest) == NULL && swMng->portPacket[i] != NULL)
		{
			*(smallest) = swMng->portPacket[i];
			portOfSmallest = i;
		}
		// else if port packet is not NULL AND a smaller port time has been found, set the smallest as the port and port of smallest to be the index of it.
		// smallest will only be checked if it's not NULL because of the condition above.
		else if ((swMng->portPacket[i] != NULL) && (swMng->portPacket[i]->time < (*(smallest))->time))
		{
			*(smallest) = swMng->portPacket[i];
			portOfSmallest = i;
		}			
	}

	return portOfSmallest;
}

/*
Function name: advancePortPacket
Description : Advances the port packet of the given portToAdvacne.
Input: Pointer to a switchManager swMng, Interger portToAdvance
Output: none
Algorithm: Checks if portToAdvance is valid - if it is, memory allocates for the port packet in the given portToAdvance..
		   .. and reads from the port in file matching the portToAdvance.
*/
void advancePortPacket(switchManager * swMng, int portToAdance)
{
	// check if portToAdvance is valid
	if (portToAdance >= 0 && portToAdance < NUM_OF_PORTS)
	{
		// allocate new memory for the packet which is about to be read
		swMng->portPacket[portToAdance] = (packet *)malloc(sizeof(packet));
		if (!swMng->portPacket[portToAdance])
		{
			printf("[ERROR] ERROR MALLOC PORT PACKET.");
			exit(1);
		}
		packet_read(swMng->pFilePortIn[portToAdance], swMng->portPacket[portToAdance]);

		// if the file has reached to an end, it means the read packet is 'garbage' and needs to be freed
		if (feof(swMng->pFilePortIn[portToAdance]))
		{
			free(swMng->portPacket[portToAdance]);
			swMng->portPacket[portToAdance] = NULL;
		}
	}
}

/*
Function name: dequeueFromMgr
Description : dequeues from the queue manager by the rules of it.
Input: Pointer to a S_Out_Qs_mgr qMgr
Output: none
Algorithm: Dequeues a packet first from the high priority queue, if it's empty - ..
		   .. then dequeues a packet from the low priority queue, if it's also empty - ..
		   .. then NULL is returned.
*/
packet * dequeueFromMgr(S_Out_Qs_mgr * qMgr)
{	

	// start with high priority
	if (!isEmptyQueue(qMgr->p0))
	{
		return deque_pkt(qMgr, PACKET_PRIORITY_HIGH);
	}

	// then low priority
	if (!isEmptyQueue(qMgr->p1))
	{
		return deque_pkt(qMgr, PACKET_PRIORITY_LOW);
	}

	return NULL;
}

/*
Function name: sendPacket
Description : Sends a packet to the port out file.
Input: Pointer to a Constant switchManager swMng, Pointer to a packet pktToSend
Output: none
Algorithm: Searches the node with the port out number through the routing table..
		   .. Searches the port out file with sending portToSend the switch manager and the node..
		   .. Writes the pktToSend to the port out file with packet_write.
*/
void sendPacket(switchManager * const swMng, packet * pktToSend)
{
	// gets the node that matches the da of the pktToSend
	S_node * node = search_route(swMng->routeTable, pktToSend->Da);

	// gets the file that matches the node
	FILE * fPortToSend = portToSend(swMng, node);

	// writes the packet to the file
	packet_write(fPortToSend, pktToSend);
}

/*
Function name: portToSend
Description : Sends a packet to the port out file.
Input: Constant Pointer to switchManager swMng, Pointer to a S_node node.
Output: Pointer to a FILE.
Algorithm: Checks the output_port inside the node, returns the port out file matching to the output_port.
*/
FILE * portToSend(const switchManager * const swMng, S_node * node)
{
	switch (node->output_port)
	{
	case OUTPUT_PORT_1:
		return swMng->pFilePortOut[OUTPUT_PORT_1-1];
		break;
	case OUTPUT_PORT_2:
		return swMng->pFilePortOut[OUTPUT_PORT_2 - 1];
		break;
	case OUTPUT_PORT_3:
		return swMng->pFilePortOut[OUTPUT_PORT_3 - 1];
		break;
	case OUTPUT_PORT_4:
		return swMng->pFilePortOut[OUTPUT_PORT_4 - 1];
		break;
	default:
		return NULL;
		break;
	}
}

/*
Function name: isMoreToQueue
Description : Checks if there's more packets to queue.
Input: Constant Pointer to switchManager swMng
Output: Boolean.
Algorithm: For each port packet, check if there's a packet that's not NULL ..
		   .. if there's, return True.
		   .. if all the port packets are NULL, return False.
*/
Bool isMoreToQueue(const switchManager * const swMng)
{
	// if there a non-null packet, return true, else return false
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{		
		if (swMng->portPacket[i])
			return True;
	}
	return False;
}

/*
Function name: closeAllFiles
Description : Closes all the files inside switch manager.
Input: Pointer to switchManager swMng
Output: none.
Algorithm: Closes the file route..
		   .. for each port, close the matching file port in.
		   .. for each port, close the matching file port out.
*/
void closeAllFiles(switchManager * swMng)
{
	// close pointer of file route
	fclose(swMng->pFileRoute);

	// close pointer of file port in
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		fclose(swMng->pFilePortIn[i]);
	}

	// close pointer of file port out
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		fclose(swMng->pFilePortOut[i]);
	}
}

/*
Function name: freeAllSwMng
Description : Frees all the mallocs inside switch manager.
Input: Pointer to switchManager swMng
Output: none.
Algorithm: Frees all the mallocs inside the routing table via deleteTree..
		   Frees all the mallocs inside the packets via freePackets..
		   Frees all the mallocs inside the queue manager via freeQueueMgr..
		   Frees the malloc of switch manager.
		   
*/
void freeAllSwMng(switchManager * swMng)
{
	// deletes and frees all the nodes inside the routing table
	deleteTree(swMng->routeTable);	

	// free all the packets inside the switch manager
	freePackets(swMng);

	// free all the queue manager
	freeQueueMgr(swMng);

	// free the pointer of the switch manager itself
	free(swMng);
}

/*
Function name: freePackets
Description : Frees all the mallocs inside the port packet.
Input: Pointer to switchManager swMng
Output: none.
Algorithm: For each port, free the packet of the matching port packet with freePacket.

*/
void freePackets(switchManager * swMng)
{
	if (swMng->portPacket == NULL)
		return;
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		freePacket(swMng->portPacket[i]);
	}
	// no need for free(swMng->portPacket) because it's const array
}

/*
Function name: freePacket
Description : Frees all the mallocs inside the packet.
Input: Pointer to packet pktToRem
Output: none.
Algorithm: Frees the mallocs for the Data filed in the packet..
		   Frees the malloc for the pktToRem.

*/
void freePacket(packet * pktToRem)
{
	// check if the pointer to the packet is valid
	if (pktToRem == NULL)
		return;
	free(pktToRem->Data);
	free(pktToRem);
}

/*
Function name: freeQueueMgr
Description : Frees all the mallocs inside the queue manager.
Input: Pointer to packet pktToRem.
Output: none.
Algorithm: Frees the mallocs for packets that might've stayed in the queue..
		   (the dequeueFromMgr should always return NULL here in a normal condition.. 
		   ..because the all the queues should be empty from packets already)..
		   For each port, Frees the malloc of the queues inside the queue manager matching the port.

*/
void freeQueueMgr(switchManager * swMng)
{
	packet * pktToRem;
	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		// while dequeueFromMgr returns a valid pointer to a packet..
		// .. continue to free the packet.
		while (pktToRem = dequeueFromMgr(swMng->qsMgr[i]))
		{
			freePacket(pktToRem);
		}
	}

	for (int i = 0; i < NUM_OF_PORTS; i++)
		freeQueues(swMng->qsMgr[i]);
}

/*
Function name: freeQueues
Description : Frees all the mallocs inside of the queue inside the queue manager.
Input: Pointer to S_Out_Qs_mgr qsMgr.
Output: none.
Algorithm: Frees the high priority and low priority queues inside the queue manager.

*/
void freeQueues(S_Out_Qs_mgr * qsMgr)
{	
	free(qsMgr->p0);
	free(qsMgr->p1);
}

/*
Function name: enqueueAllPackets
Description : Reads all the packets inside the port in files, and enqueues them inside the matching port out queue manager sorted by time.
Input: Pointer to switchManager swMng.
Output: none.
Algorithm: Reads a single packet from each port file, finds the smallest time from them and enqueues it in the matching port out queue manager..
		   .. Which is found via the routing table using the da and output port number..
		   .. After that, the port of the smallest packet gets a new packet from the port in file.
		   .. This continues until there's no more packets left to read from the port in file (All the matching port packets are NULL).

*/
void enqueueAllPackets(switchManager * swMng)
{
	int portOfSmallest;
	packet * smallestPacket = NULL;
	S_node * routeNode;
	while (isMoreToQueue(swMng))
	{
		// receive the packet to read
		portOfSmallest = smallestTimePacket(swMng, &smallestPacket);

		// the loop should only be entered if there's still packets to read
		if (portOfSmallest == PORT_INIT_VALUE || smallestPacket == NULL)
		{
			printf("[ERROR] TRYING TO RECEIEVE A PACKET WHEN ALL PACKET PORTS ARE NULL");
			exit(1);
		}

		// gets the route node of that matches the da of the received packet.
		routeNode = search_route(swMng->routeTable, smallestPacket->Da);

		// check if there's a route to the Destination Address written inside the packet,..
		// ..check if the output port is a valid port
		if (routeNode == NULL || (routeNode->output_port < 0 || routeNode->output_port > NUM_OF_PORTS))
		{
			free(smallestPacket);
		}
		else
		{
			// enqueue the packet the smallest time packet
			enque_pkt(swMng->qsMgr[routeNode->output_port - 1], smallestPacket);
		}

		// advance the port from which the smallest packet came from
		advancePortPacket(swMng, portOfSmallest);

		smallestPacket = NULL;
	}
}

/*
Function name: dequeueAllPackets
Description : Dequeues all the packets from the queue managers and writes them in the matching port out files.
Input: Pointer to switchManager swMng.
Output: none.
Algorithm: For each port, dequeue all the packets in the queue manager and send them to the matching port out..
		   .. After the packet has been sent, free it from memory.

*/
void dequeueAllPackets(switchManager * swMng)
{
	packet * pktToSend;

	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		while (!isEmptyQueue(swMng->qsMgr[i]->p0))
		{
			// receive a packet from the Queue Manager
			pktToSend = deque_pkt(swMng->qsMgr[i], PACKET_PRIORITY_HIGH);

			// Transfer the packet to the destination port.
			sendPacket(swMng, pktToSend);

			// Free the processed packet
			free(pktToSend);
		}
	}

	for (int i = 0; i < NUM_OF_PORTS; i++)
	{
		while (!isEmptyQueue(swMng->qsMgr[i]->p1))
		{
			// receive a packet from the Queue Manager
			pktToSend = deque_pkt(swMng->qsMgr[i], PACKET_PRIORITY_LOW);

			// Transfer the packet to the destination port.
			sendPacket(swMng, pktToSend);

			// Free the processed packet
			free(pktToSend);
		}
	}
}