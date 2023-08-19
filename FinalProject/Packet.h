#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>

#define PACKET_PRIORITY_HIGH 0
#define PACKET_PRIORITY_LOW 1
#define PACKET_DATA_MIN_SIZE 1
#define PACKET_DATA_MAX_SIZE 255
#define NODE_NOT_FOUND NULL

#define CMD_ADD 'a'
#define CMD_DEL 'd'

typedef struct packet {
	unsigned int time;
	unsigned char Da;
	unsigned char Sa;
	char Prio;
	char Data_Length;
	unsigned char * Data;
	unsigned char Checksum;
}packet;

typedef enum Bool {True = 1, False = 0} Bool;

typedef struct route_node {
	unsigned char da;
	char output_port;
	struct route_node *left;
	struct route_node *right;
} S_node;

typedef struct command_struct {
	char command;
	unsigned char Da;
	char output_port;
}command_struct;

typedef struct pkt_node {
	packet *pkt;
	struct pkt_node *next;
} pkt_node;

typedef struct Queue {
	struct pkt_node *head;
	struct pkt_node *tail;
}Queue;

typedef struct Out_Qs_mgr {
	struct Queue * p0;
	struct Queue * p1;
} S_Out_Qs_mgr;

void packet_read(FILE *fp, packet *pkt);
void packet_write(FILE *fp, const packet *pkt);
int scanFormatedPacket(FILE * const fp, const char * format, packet * ptr);
int scanFormatedData(FILE * const fp, const char * format, unsigned char * ptr);
int scanFormatedCmd(FILE * const fp, const char * format, command_struct * ptr);
Bool checksum_check(const packet *pkt);
unsigned char calculateChecksum(const packet * pkt);
S_node * add_route(S_node *root, unsigned char da, char output_port);
S_node *delete_route(S_node *root, unsigned char da);
S_node * smallestLeaf(S_node * root);
S_node *search_route(const S_node *root, unsigned char da);
void print_routing_table(const S_node *root);
S_node * build_route_table(FILE *fp, S_node *root);
Bool isEmptyQueue(Queue * q);
void enqueue(Queue * q, packet * toAdd);
packet * dequeue(Queue * q);
void enque_pkt(S_Out_Qs_mgr *QM_ptr, packet *pkt);
packet *deque_pkt(S_Out_Qs_mgr *QM_ptr, char priority);
void deleteTree(S_node * root);
#endif