#define _CRT_SECURE_NO_WARNINGS
#include "Packet.h"
#include <stdio.h>
#include <stdlib.h>

/*
Function name: packet_read
Description : Reads the packets from the files.
Input: Pointer to a FILE, Pointer to a packet
Output: none
Algorithm: Allocates memory for Data, reads the lines from file
*/
void packet_read(FILE *fp, packet *pkt)
{
	// Check if pointers are valid.
	if (!fp)
	{
		printf("[ERROR] NULL FILE fp pointer.");
		pkt = NULL;
		return;
	}
	if (!pkt)
	{
		printf("[ERROR] NULL packet pkt pointer.");
		pkt = NULL;
		return;
	}

	// Read from file: time, da, sa, prio and Data_Length.
	if(scanFormatedPacket(fp, "%d %d %d %d %d", pkt) != EOF)
	{
		// Check if Data_Length is valid.
		if (pkt->Data_Length < PACKET_DATA_MIN_SIZE || pkt->Data_Length > PACKET_DATA_MAX_SIZE)
		{
			printf("[ERROR] Packet's Data size is not valid [1-255].");
			pkt = NULL;
			return;
		}


		pkt->Data = (unsigned char *)malloc(sizeof(unsigned char) * pkt->Data_Length);

		// Check if malloc succeeded 
		if (!pkt->Data)
		{
			printf("[ERROR] Packet's Data memory allocation failed.");
			pkt = NULL;
			return;
		}

		// Read from file: all of the data bytes.
		for (int i = 0; i < pkt->Data_Length; i++)
			scanFormatedData(fp, "%d", &(pkt->Data[i]));

		// Read from file: Checksum.
		scanFormatedData(fp, "%d", &(pkt->Checksum));		
	}
	else
	{
		pkt = NULL;
	}
}

/*
Function name: scanFormatedPacket
Description : Scans from the pointed file using the format.
Input: Pointer to File, Pointer to char, Pointer to a packet
Output: returns Integer.
Algorithm: Uses fscanf with the given file fp and string format..
..		   into Interger holder variables, then uses casting to unsigned char..
..		   into the pointer of the packet's matching fields.
..		   Returns the call result of the fscanf function.
*/
int scanFormatedPacket(FILE * const fp, const char * format, packet * ptr)
{
	unsigned int tempDa;
	unsigned int tempSa;
	unsigned int tempPrio;
	unsigned int tempDataLength;	
	int ret = fscanf(fp, format, &(ptr->time), &(tempDa), &(tempSa), &(tempPrio), &(tempDataLength));
	ptr->Da = (unsigned char)tempDa;
	ptr->Sa = (unsigned char)tempSa;
	ptr->Prio = (signed char)tempPrio;
	ptr->Data_Length = (signed char)tempDataLength;
	return ret;
}

/*
Function name: scanFormatedData
Description : Scans from the pointed file using the format.
Input: Pointer to a const FILE, Pointer to a const format of chars, Pointer to an unsigned char
Output: Returns Integer.
Algorithm: Uses fscanf with the given file fp and string format..
..		   into Interger holder variable, then uses casting to unsigned char..
..		   into the given pointer of the char.
..		   Returns the call result of the fscanf function.
*/
int scanFormatedData(FILE * const fp, const char * format, unsigned char * ptr)
{
	unsigned int tempData;
	int ret = fscanf(fp, format, &(tempData));
	*(ptr) = (unsigned char)tempData;
	return ret;
}

/*
Function name: packet_write
Description : Writes into file the values of the packet
Input: Pointer to file, pointer to format of chars ,pointer to packet
Output: none
Algorithm: checks if everything is valid if not it returns,.. 
..		   After valid check succeeded, it prints the packet's values to the matching file
*/
void packet_write(FILE *fp, const packet *pkt)
{
	// Check if pointers are valid.
	if (!fp)
	{
		printf("[ERROR] NULL FILE fp pointer.");		
		return;
	}
	if (!pkt)
	{
		printf("[ERROR] NULL packet pkt pointer.");		
		return;
	}

	// Check if Data_Length is valid.
	if (pkt->Data_Length < PACKET_DATA_MIN_SIZE || pkt->Data_Length > PACKET_DATA_MAX_SIZE)
	{
		printf("[ERROR] Packet's Data size is not valid [1-255].");		
		return;
	}

	// Check if Data pointer is valid
	if (!pkt->Data)
	{
		printf("[ERROR] Packet's Data pointer is NULL.");		
		return;
	}

	// Write to file: time, da, sa, prio and Data_Length.
	fprintf(fp, "%d %d %d %d %d ", (pkt->time), (pkt->Da), (pkt->Sa), (pkt->Prio), (pkt->Data_Length));

	// Write to file: all of the data bytes.
	for (int i = 0; i < pkt->Data_Length; i++)
		fprintf(fp, "%d ", (pkt->Data[i]));

	// Write to file: Checksum.
	fprintf(fp, "%d\n", (pkt->Checksum));
}

/*
Function name: checksum_check
Description : Checks if the packet's checksum is valid
Input: Pointer to a packet
Output: Boolean
Algorithm: First it checks if the given packet is non-null,..
..		   Then it checks if the data length is valid,..
..		   Then it calls calculateChecksum to calculate the packet's checksum,..
..		   and it compares it to the checksum field inside the packet.
*/
Bool checksum_check(const packet *pkt)
{
	// Check if pointers are valid.
	if (!pkt)
	{
		printf("[ERROR] NULL packet pkt pointer.");
		return False;
	}

	// Check if Data_Length is valid.
	if (pkt->Data_Length < PACKET_DATA_MIN_SIZE || pkt->Data_Length > PACKET_DATA_MAX_SIZE)
	{
		printf("[ERROR] Packet's Data size is not valid [1-255].");
		return False;
	}

	// Check if Packet's Checksum field is equals to ..
	//..the calculated checksum of the packet. 
	if (pkt->Checksum == calculateChecksum(pkt))
		return True;

	return False;
}

/*
Function name: calculateChecksum
Description : Calculates the Checksum of the given packet
Input: Pointer to packet
Output: unsigned char
Algorithm: Uses XORs on all of the packet's monitored fields,..
..		   Which are:
..		   a) Da
..		   b) Sa
..		   c) Prio
..		   d) Data_Length
..		   e) Data
..
..		   Then returns that calculated value.
*/
unsigned char calculateChecksum(const packet * pkt)
{
	// init calcChecksum as Da.
	unsigned char calcChecksum = pkt->Da;

	// XOR calcChecksum with Sa.
	calcChecksum ^= pkt->Sa;

	// XOR calcChecksum with Prio.
	calcChecksum ^= pkt->Prio;

	// XOR calcChecksum with Data_Length.
	calcChecksum ^= pkt->Data_Length;

	// XOR calcChecksum with Data.
	for (int i = 0; i < pkt->Data_Length; i++)
	{
		calcChecksum ^= pkt->Data[i];
	}

	return calcChecksum;
}

/*
Function name: add_route
Description : Adds a route to the given tree root
Input: Pointer to a S_node, unsigned char, char
Output: Pointer to a S_node
Algorithm: Allocates memory for a new node,..
..		   If the allocation succeeded, it puts the given da and output_port inside the new node. ..
..		   If root is NULL, then set temp as the root of the tree and return root.
..		   Else The function then finds the correct place to insert the new node using.. 
..		   the algorithm of Binary Search Tree: 
..		   left sub-tree is less or equal to the parent's tree value,.. 
..		   right sub-tree is greater than the parent's tree value.
*/
S_node * add_route(S_node *root, unsigned char da, char output_port)
{
	S_node * temp = (S_node *)malloc(sizeof(S_node));
	// Check if malloc succeeded 
	if (!temp)
	{
		printf("[ERROR] Node's memory allocation failed.");
		return root;
	}
	temp->da = da;
	temp->output_port = output_port;
	temp->left = NULL;
	temp->right = NULL;

	// root is null
	if (root == NULL)
	{
		root = temp;
		return root;
	}

	// root is not null
	S_node * ptr = root;
	while (ptr)
	{
		// move left
		if (temp->da <= ptr->da)
		{
			// check if left exists
			if (ptr->left)
			{
				ptr = ptr->left;
			}
			
			// insert the node in the left of ptr
			else
			{
				ptr->left = temp;
				return root;
			}
		}

		// move right
		else
		{
			// check if right exists
			if (ptr->right)
			{
				ptr = ptr->right;
			}

			// insert the node in the right of ptr
			else
			{
				ptr->right = temp;
				return root;
			}
		}
	}

	// never happens because it will return beforehand
	return NULL;
}

/*
Function name: delete_route
Description : Deletes a route from root which has the given da
Input: Pointer to a S_node, unsigned char
Output: Pointer to a S_node
Algorithm: The function searches for a node with the given da value using the BST algorithm,..
..		   the algorithm of Binary Search Tree:
..		   left sub-tree is less or equal to the parent's tree value,..
..		   right sub-tree is greater than the parent's tree value.
..		   if a node is found, set the parent of the found node to now point at the right's sub-tree of the found node. ..
..		   if the found node has left sub-tree, set the smallest leaf of the found node to now point at that sub-tree.
..		   in this method, we cover all the cases for deleting a node without losing any of its sub-trees (if it has).
..		   then return the root of the updated tree.
*/
S_node *delete_route(S_node *root, unsigned char da)
{

	// root is null
	if (root == NULL)
	{
		return root;
	}

	// root is not null and there is only root
	if (root->left == NULL && root->right == NULL)
	{
		// check if root's da matches da
		if (root->da == da)
		{
			// delete root
			free(root);
			return NULL;
		}
		else
		{
			// no da found
			return root;
		}
	}

	// root is not null
	S_node * ptrPrev = NULL;
	S_node * ptr = root;
	S_node * temp;
	while (ptr)
	{
		// move left
		if (da < ptr->da)
		{
			// check if left exists
			if (ptr->left)
			{
				ptrPrev = ptr;
				ptr = ptr->left;
			}

			// no da found
			else
			{
				return root;
			}
		}

		// move right
		else if (da > ptr->da)
		{
			// check if right exists
			if (ptr->right)
			{
				ptrPrev = ptr;
				ptr = ptr->right;
			}

			// no da found
			else
			{
				return root;
			}
		}

		// da and ptr's da match has been found
		else
		{
			// delete a leaf
			if (ptr->left == NULL && ptr->right == NULL)
			{
				free(ptr);
				if (da > ptrPrev->da)
					ptrPrev->right = NULL;
				else
					ptrPrev->left = NULL;
			}

			// delete a middle node

			// check which direction we took from the parent
			if (ptr->da > ptrPrev->da)
			{
				// set parent right to the right of the deleted node
				ptrPrev->right = ptr->right;
			}
			else
			{
				// set parent left to the right of the deleted node
				ptrPrev->left = ptr->right;
			}

			// check if we need to move the left's nodes
			if (ptr->left)
			{
				// get the smallest leaf of the right's deleted node
				temp = smallestLeaf(ptr->right);

				// set the left's node of the smallest leaf of the right's deleted node to..
				//..be the parent of the left's deleted node.
				temp->left = ptr->left;
			}

			free(ptr);
			return root;
		}
	}

	return root;
}

/*
Function name: smallestLeaf
Description : Finds the smallest leaf in the given tree
Input: Pointer to a S_node
Output: Returns the node of the smallest value in the tree.
Algorithm: Use the BST algorithm to find the smallest value in the tree,..
..		   the algorithm of Binary Search Tree:
..		   left sub-tree is less or equal to the parent's tree value,..
..		   right sub-tree is greater than the parent's tree value.
..		   which means, the smallest value is the most left valid node in the tree.
*/
S_node * smallestLeaf(S_node * root)
{
	if (!root)
		return NULL;

	S_node * temp = root;
	while (temp->left != NULL)
		temp = temp->left;

	return temp;
}

/*
Function name: search_route
Description : Searches a node from root which has the given da
Input: Pointer to a S_node, unsigned char
Output: Pointer to a S_node
Algorithm: The function searches for a node with the given da value using the BST algorithm,..
..		   the algorithm of Binary Search Tree:
..		   left sub-tree is less or equal to the parent's tree value,..
..		   right sub-tree is greater than the parent's tree value.
..		   in this method, we cover all the cases for searching a node inside the tree.
..		   if a node is found, return the found node,..
..		   else return NODE_NOT_FOUND (NULL)
*/
S_node *search_route(const S_node *root, unsigned char da)
{
	// root is null
	if (root == NULL)
	{
		return root;
	}

	// root is not null
	S_node * ptr = root;
	while (ptr)
	{
		// move left
		if (da < ptr->da)
		{
			// check if left exists
			if (ptr->left)
			{
				ptr = ptr->left;
			}

			// insert the node in the left of ptr
			else
			{
				return NODE_NOT_FOUND;
			}
		}

		// move right
		else if(da > ptr->da)
		{
			// check if right exists
			if (ptr->right)
			{
				ptr = ptr->right;
			}

			// insert the node in the right of ptr
			else
			{
				return NODE_NOT_FOUND;
			}
		}

		// match found
		else
		{
			return ptr;
		}
	}

	// never happens because it will return beforehand
	return NULL;
}

/*
Function name: print_routing_table
Description : Prints all the nodes' values (DA and Output Port) of the given tree.
Input: Pointer to a S_node
Output: none
Algorithm:  Recursively run through the tree using the 'InOrder' searching method,..
..			For each node, print the Destination Address and Output port values inside of it.

*/
void print_routing_table(const S_node *root)
{
	if (!root)
		return;
	print_routing_table(root->left);
	printf("Node Destination Address: %d ; Node Output Port: %d\n", root->da, root->output_port);
	print_routing_table(root->right);
}

/*
Function name: build_route_table
Description : Builds a routing table using the commands written in the given File.
Input: Pointer to a File , Pointer to a S_node
Output: Pointer to a S_node
Algorithm: First checks if the input is valid, if so then memory allocation is created for the command struct,..
..		   Starts to build the tree by reading commands from the given File line by line using the command struct.
..		   If the command is 'a' - add:
..		   A call is being made for add_route with the appropriate values and the given root node.
..		   Else if the command is 'd' - delete:
..		   A call is being made for delete_route with the appropriate values and the given root node.
..		   Else, the given command is not valid.
..		   After all commands are executed, the function returns the root of the builded tree.
*/
S_node * build_route_table(FILE *fp, S_node *root)
{
	if (!fp)
		return NULL;

	command_struct * temp = (command_struct *)malloc(sizeof(command_struct));
	if (!temp)
	{
		printf("[ERROR] Command Structure memory allocation failed.");
		return NULL;
	}

	printf("Starting to Run Building Route Table Script File...\n");	
	while (scanFormatedCmd(fp, "%c %d %d\n", temp) != EOF)
	{
		switch (temp->command)
		{
		case CMD_ADD:
			root = add_route(root, temp->Da, temp->output_port);
			break;
		case CMD_DEL:
			root = delete_route(root, temp->Da);
			break;
		default:
			printf("[ERROR] BUILD_ROUTE_TABLE: WRONG COMMAND\n");
			break;
		}
	}

	printf("Finished Running Building Route Table Script File!\n");

	//print_routing_table(root);	
	free(temp);
	return root;
}

/*
Function name: scanFormatedCmd
Description : Scans from the pointed file using the format.
Input: Pointer to a const FILE, Pointer to a const format of chars, Pointer to a command_struct
Output: Returns Integer.
Algorithm: Uses fscanf with the given file fp and string format..
..		   into Interger holder variable, then uses casting to unsigned char..
..		   into the given pointer of the command_struct's matching fields.
..		   Returns the call result of the fscanf function.
*/
int scanFormatedCmd(FILE * const fp, const char * format,command_struct * ptr)
{
	unsigned int tempDa;
	unsigned int tempOutputPort;
	int ret = fscanf(fp, format, &(ptr->command), &(tempDa), &(tempOutputPort));
	ptr->Da = (unsigned char)tempDa;
	ptr->output_port = (unsigned char)tempOutputPort;
	return ret;
}

/*
Function name: isEmptyQueue
Description : Checks if the queue is empty
Input: Pointer to a queue
Output: Boolean
Algorithm: Checks if the queue's head or tail fields are nulls,..
..		   If any of them are NULL, return True.
..		   Else return False;
*/
Bool isEmptyQueue(Queue * q)
{
	if (q->head == NULL || q->tail == NULL)
		return True;
	return False;
}

/*
Function name: enqueue
Description : Enqueue a given packet into the given queue
Input: Pointer to a queue, Pointer to a packet
Output: none
Algorithm: Allocates memory for a new node,..
..		   If the queue is empty, set the new node as the head and tail of the queue.
...		   Else set the current tail's next as the new node,..
..		   and then set the tail itself as the new node - that way the new node is added to the tail of the queue.
*/
void enqueue(Queue * q, packet * toAdd)
{
	pkt_node *newNode = (struct pkt_node *)malloc(sizeof(pkt_node));
	if (!newNode)
	{
		printf("[ERROR] New Node memory allocation failed.");
		return;
	}
	newNode->pkt = toAdd;
	newNode->next = NULL;
	if (q->head == NULL)
	{
		q->head = newNode;
		q->tail = q->head;
	}
	else
	{
		q->tail->next = newNode;
		q->tail = newNode;
	}
}

/*
Function name: dequeue
Description : Dequeue a packet from the given queue
Input: Pointer to a queue
Output: Pointer to a packet
Algorithm: Checks if the queue is invalid or empty,
..		   If it is, then return NULL.
..		   Else, dequeue the head of the list and set the new head as the next's node.
..		   If the updated list is empty, then set tail to be NULL as well.
..		   Returns the dequeued packet.
*/
packet * dequeue(Queue * q)
{
	// check if the queue is invalid or empty
	if (q == NULL || q->head == NULL)
		return NULL;


	pkt_node *tmp = q->head;
	packet * tmpPkt = NULL;

	// dequeue the head
	tmpPkt = tmp->pkt;
	q->head = q->head->next;		
	free(tmp);
	if (q->head == NULL)
		q->tail = q->head;

	return tmpPkt;
}

/*
Function name: enque_pkt
Description : Dequeue a packet to the queue manager.
Input: Pointer to a S_Out_Qs_mgr QM_ptr, Pointer to a packet pkt
Output: none
Algorithm: First, checks if the checksum in the pkt matches the calculated checksum via the function checksum_check..
..		   high priority will enqueue the packet to the high priority queue,
..		   low priority will enqueue the packet to the low priority queue.
*/
void enque_pkt(S_Out_Qs_mgr *QM_ptr, packet *pkt)
{	
	// checking if the packet is valid via checksum calc.
	if (!checksum_check(pkt))
	{
		free(pkt);
		return;
	}

	switch (pkt->Prio)
	{
	case PACKET_PRIORITY_HIGH:
		enqueue(QM_ptr->p0, pkt);
		break;
	case PACKET_PRIORITY_LOW:
		enqueue(QM_ptr->p1, pkt);
		break;
	default:
		// priority is invalid
		free(pkt);		
		break;
	}
}

/*
Function name: deque_pkt
Description : Dequeue a packet from the queue manager using the given priority.
Input: Pointer to a S_Out_Qs_mgr QM_ptr, Char priority
Output: Pointer to a packet
Algorithm: Dequeues a packet from the queue manager matching the priority - ..
..		   high priority will dequeue a packet from the high priority queue,
..		   low priority will dequeue a packet from the low priority queue.
*/
packet *deque_pkt(S_Out_Qs_mgr *QM_ptr, char priority)
{

	switch (priority)
	{
	case PACKET_PRIORITY_HIGH:
		return dequeue(QM_ptr->p0);
		break;
	case PACKET_PRIORITY_LOW:
		return dequeue(QM_ptr->p1);
		break;
	default:		
		return NULL;
		break;
	}
}

/*
Function name: deleteTree
Description : Deletes all the nodes inside the tree.
Input: Pointer to a node root
Output: none
Algorithm: Recursive run using the 'PostOrder' delete method,.. 
..		   Deletes all the left nodes, then all the right nodes, then deletes itself.
*/
void deleteTree(S_node * root)
{
	if (!root)
		return;
	deleteTree(root->left);
	deleteTree(root->right);
	free(root);
}