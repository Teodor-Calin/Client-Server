#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int main(int argc, char *argv[])
{
    char *command;
    int sockfd;
    char* address = "34.118.48.238";
    char *books_route = "/api/v1/tema/library/books";

    command = malloc(100 * sizeof(char));
    strcpy(command, "");

    char *message = malloc(500 * sizeof(char));
    char *response = malloc(500 * sizeof(char));
    char *token = malloc(500 * sizeof(char));
    strcpy(token, " ");
    char *cookie = malloc(500 * sizeof(char));
    strcpy(cookie, " ");


    while(strcmp(command, "exit") != 0) {
        scanf("%s", command);

        if (strcmp(command, "register") == 0) {
            char *username = malloc(50 * sizeof(char));
            char *password = malloc(50 * sizeof(char));

            // register prompt
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);

            // json object creation and converting it into a string
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            char * x = json_serialize_to_string_pretty(value);


            // compute massage, send it and receive response
            message = compute_post_request(address, "/api/v1/tema/auth/register", "application/json", &x, 1, NULL, 0, NULL);

            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);
            printf("\n");

            char *aux = strstr(response, "error");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                puts(response);
            } else {
                printf("Account created!\n");
            }

        } else if (strcmp(command, "login") == 0) {

            char *username = malloc(50 * sizeof(char));
            char *password = malloc(50 * sizeof(char));

            // login prompt
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);

            // json object creation and converting it into a string
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);
            char *x = json_serialize_to_string_pretty(value);

            // compute massage, send it and receive response
            message = compute_post_request(address, "/api/v1/tema/auth/login", "application/json", &x, 1, NULL, 0, NULL);

            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);
            printf("\n");

            // keep the cookie in a variable
            char* aux = strstr(response, "connect");
            if (aux != NULL) {
                strcpy(response, aux);
                aux = strstr(response, ";");
                int nr_c = aux - response;
                strncpy(cookie, response, nr_c);
                strcpy(token, "");
                printf("You logged in!\n");
            } else {
                char *aux = strstr(response, "error");
                if (aux != NULL) {
                    strcpy(response, aux + 8);
                    response[strlen(response) - 2] = '\0';
                    puts(response);
                }
            }
            
            free(username);
            free(password);

        } else if (strcmp(command, "enter_library") == 0) {

            // compute massage, send it and receive response
            message = compute_get_request(address, "/api/v1/tema/library/access", NULL, &cookie, 1, NULL);
            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);
            printf("\n");

            char *aux = strstr(response, "token");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                strcpy(token, response);
                printf("You got access to the library!\n");
            } else {
                aux = strstr(response, "error");
                if (aux != NULL) {
                    strcpy(response, aux + 8);
                    response[strlen(response) - 2] = '\0';
                    puts(response);
                }
            }

        } else if (strcmp(command, "get_books") == 0) {
            
            // compute massage, send it and receive response
            message = compute_get_request(address, books_route, NULL, &cookie, 1, token);
            
            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);
            printf("\n");

            char *aux = strstr(response, "error");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                puts(response);
            }

        } else if (strcmp(command, "get_book") == 0) {

            
            // book id prompt
            char *book_number = malloc(50 * sizeof(char));
            printf("id=");
            scanf("%s", book_number);

            // create route for specified book id
            char *book_aux = malloc(100 * sizeof(char));
            strcpy(book_aux, books_route);
            strcat(book_aux, "/");
            strcat(book_aux, book_number);

            // compute massage, send it and receive response
            message = compute_get_request(address, book_aux, NULL, &cookie, 1, token);
            
            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);

            char *aux = strstr(response, "error");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                puts(response);
            }

            free(book_number);
            free(book_aux);

        } else if (strcmp(command, "add_book") == 0) {


            char* title = malloc(100 * sizeof(char));
            char* author = malloc(100 * sizeof(char));
            char* genre = malloc(30 * sizeof(char));
            char* publisher = malloc(100 * sizeof(char));
            char* page_count = malloc(10 * sizeof(char));

            // book prompt
            printf("title=");
            scanf("%s", title);
            printf("author=");
            scanf("%s", author);
            printf("genre=");
            scanf("%s", genre);
            printf("publisher=");
            scanf("%s", publisher);
            printf("page_count=");
            scanf("%s", page_count);

            if (atoi(page_count) <= 0) {
                printf("This field must be a positive integer!\n");
                continue;
            }
            
            // json object creation and converting it into a string
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "title", title);
            json_object_set_string(object, "author", author);
            json_object_set_string(object, "genre", genre);
            json_object_set_string(object, "publisher", publisher);
            json_object_set_string(object, "page_count", page_count);
            
            char *x = json_serialize_to_string_pretty(value);

            // compute massage, send it and receive response
            message = compute_post_request(address, books_route, "application/json", &x, 1, &cookie, 1, token);
            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);

            char *aux = strstr(response, "error");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                puts(response);
            }

            free(title);
            free(author);
            free(genre);
            free(publisher);
            free(page_count);

        } else if (strcmp(command, "delete_book") == 0) {

            char* book_number = malloc(100 * sizeof(char));

            // book id prompt
            printf("id=");
            scanf("%s", book_number);
            
            // create route for specified  book id
            char *book_aux = malloc(120 * sizeof(char));
            strcpy(book_aux, books_route);
            strcat(book_aux, "/");
            strcat(book_aux, book_number);

            // compute massage, send it and receive response
            message = compute_delete_request(address, book_aux, NULL, &cookie, 1, token);
            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);

            char *aux = strstr(response, "error");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                puts(response);
            }

            free(book_aux);
            free(book_number);

        } else if (strcmp(command, "logout") == 0) {

            // compute massage, send it and receive response
            message = compute_get_request(address, "/api/v1/tema/auth/logout", NULL, &cookie, 1, NULL);
            sockfd = open_connection(address, 8080, AF_INET, SOCK_STREAM, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            close(sockfd);
            puts(response);
            printf("\n");
            
            char *aux = strstr(response, "error");
            if (aux != NULL) {
                strcpy(response, aux + 8);
                response[strlen(response) - 2] = '\0';
                puts(response);
            } else {
                printf("You successfully logged out!\n");
            }

            // clear cookie and token
            strcpy(cookie, " ");
            strcpy(token, " ");
        }
    }

        
    

    return 0;
}
