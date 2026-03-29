#ifndef SERVER_HANDLES_H
#define SERVER_HANDLES_H

typedef struct Server Server;
typedef struct Client Client;

int handle_connection(Client *client);
int handlers_dispatch(Client *c);
int handle_auth_login(Client *c);
int handle_file_send(Server *s, Client *c);
int handle_inbox_list(Server *s, Client *c);
int handle_inbox_accept(Server *s, Client *c);
int handle_inbox_decline(Server *s, Client *c);

#endif
