#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 16777216 
#define MAX_OBJECT_SIZE 8388608 

struct Request
{
	char url[MAXLINE], host[MAXLINE], path[MAXLINE], host_hdr[MAXLINE];//, other_hdr[MAXLINE];
	char port[6];	//server port	
};

typedef struct Request Request;

struct Node
{
	char url[MAXLINE];
	void *item;
	size_t size;
	struct Node *prev, *next;
};

struct List
{
	size_t size;
	struct Node *first, *last;
};

struct List list;

//////////////////////List Interface///////////////////////////////
void initialise_cache()
{
	list.size = 0;
	list.first = list.last = NULL;
}

void evict()
{
	while(list.size > MAX_CACHE_SIZE)
	{
		struct Node *victime = list.last;
		list.size -= victime->size;
		list.last = victime->prev;
		list.last->next = NULL;
		Free(victime->item);
		Free(victime);
	}
}

void store(char *url, void *item, size_t size)
{
	list.size += size;
	if(list.size > MAX_CACHE_SIZE)
		evict();
	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
	strcpy(node->url, url);
	node->item = item;
	node->size = size;
	node->prev = node->next = NULL;
	if(list.first)
	{
		node->next = list.first;
		list.first->prev = node;
	}
	else
		list.last = node;
	list.first = node;
}

struct Node *find(char *url)
{
	struct Node *p = list.first;
	for(; p ; p = p->next)
		if(strcmp(p->url, url) == 0)
			return p;
	return NULL;
}

void move_to_front(char *url)
{
	struct Node *node = find(url);
	if(!node)
		return;
	if(!node->prev)
		return;
	if(node->next)
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	else
	{
		node->prev->next = NULL;
		list.last = node->prev;
	}
	node->prev = NULL;
	node->next = list.first;
	list.first = node;
	if(node->next)
		node->next->prev = node;
	else
		list.last = node;
}

//create and initialise a new Request object, return it's reference
Request* create_request()
{
	Request *r = (Request *)malloc(sizeof(Request));
	strcpy(r->url, "");
	strcpy(r->host, "");
	strcpy(r->port, "80");
	strcpy(r->path, "/");
	return r;
}

void parse_request(char line[MAXLINE], Request *r)
{
	char temp[10];
	sscanf(line, "%s %s %s", temp, r->url, temp);
	printf("URL: %s\n", r->url);
	char url[MAXLINE];
	strcpy(url, r->url);
	char *saved, *host, *port, *path;
	strtok_r(url, "/", &saved);
	port = strtok_r(saved, "/", &path);
	host = strtok_r(port, ":", &port);
	strcat(r->host, host);
	strcat(r->path, path);
	strcat(r->host_hdr, "Host: ");
	strcat(r->host_hdr, host);
	strcat(r->host_hdr, "\r\n");
	strcpy(r->port, port);
}

void concate(Request *r, char *request)
{
	strcpy(request, "GET ");
	strcat(request, r->path);
	strcat(request, " HTTP/1.0\r\n");
	strcat(request, r->host_hdr);
	strcat(request, "\r\n");
}

void get_from_server(Request *req, char request[MAXLINE], int client, rio_t rio_to_client)
{
	char *buf = Malloc(MAXLINE);
  	char *cache_buf = Calloc(1, MAX_CACHE_SIZE);
  	char *p = cache_buf; 
  	int n, total_size = 0, can_cache = 1;

  	//connect to web server(tiny)
 	int server = open_clientfd(req->host, req->port);
  
  	rio_t rio_to_server;
  	rio_readinitb(&rio_to_server, server);
  	rio_writen(server, request, strlen(request));
  
  	// read response; forward it to client
 	while ((n = rio_readnb(&rio_to_server, buf, MAXLINE)) != 0)
  	{
    	rio_writen(client, buf, n);
		total_size += n;
		if(total_size > MAX_CACHE_SIZE)
			can_cache = 0;
		else
		{
			memcpy(p, buf, n);
			p += n;
		}
	}
	if(can_cache)
		store(req->url, cache_buf, total_size);
  	close(server);
  	Free(buf);
}

int get_from_cache(Request *r, int client)
{
	struct Node *item = find(r->url);
	if(item)
	{
		rio_writen(client, item->item, item->size);
		move_to_front(r->url);
		return 1;
	}
	return 0;
}

void handle_client(int fd)
{
	char request[MAXLINE], new_request[MAXLINE];
	Request *req = create_request();
	rio_t rio_to_client;
	rio_readinitb(&rio_to_client, fd);
	rio_readlineb(&rio_to_client, request, MAXLINE);
	parse_request(request, req);
	concate(req, new_request);
	if(!get_from_cache(req, fd))
		get_from_server(req, new_request, fd, rio_to_client);
	close(fd);
}

int main(int argc, char *argv[])
{
  	if(argc != 2){
		printf("Usage: %s [port]\n", argv[0]);
		exit(-1);
	}
	initialise_cache();
	int listenfd = open_listenfd(argv[1]);
	while(1){
		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);
		int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
		handle_client(clientfd);
	}
    return 0;
}
