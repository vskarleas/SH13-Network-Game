#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdbool.h> // CODE ADDED
#include <time.h>	 // CODE ADDED

#include <netdb.h>
#include <arpa/inet.h>

struct _client
{
	char ipAddress[40];
	int port;
	char name[40];
} tcpClients[4];
int nbClients;
int fsmServer;
int deck[13] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
int tableCartes[4][8];
char *nomcartes[] =
	{"Sebastian Moran", "irene Adler", "inspector Lestrade",
	 "inspector Gregson", "inspector Baynes", "inspector Bradstreet",
	 "inspector Hopkins", "Sherlock Holmes", "John Watson", "Mycroft Holmes",
	 "Mrs. Hudson", "Mary Morstan", "James Moriarty"};
int joueurCourant;

int liste_joueurs_elimines[4]; // code added

void init_joueurs_elimines() // CODE ADDED
{
	int i;
	for (i = 0; i < 4; i++)
		liste_joueurs_elimines[i] = 0; // 0 means that a user is not eliminated otherwise he is eliminated (his guess was wrong)
}

bool all_users_are_eliminated(int *list_users) // retruns true if all users are equal to 1
{
	int i;
	for (i = 0; i < 4; i++)
		if (list_users[i] == 0)
			return false;
	return true;
}

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void melangerDeck()
{
	int i;
	int index1, index2, tmp;

	for (i = 0; i < 1000; i++)
	{
		index1 = rand() % 13;
		index2 = rand() % 13;

		tmp = deck[index1];
		deck[index1] = deck[index2];
		deck[index2] = tmp;
	}
}

void createTable()
{
	// Le joueur 0 possede les cartes d'indice 0,1,2
	// Le joueur 1 possede les cartes d'indice 3,4,5
	// Le joueur 2 possede les cartes d'indice 6,7,8
	// Le joueur 3 possede les cartes d'indice 9,10,11
	// Le coupable est la carte d'indice 12
	int i, j, c;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 8; j++)
			tableCartes[i][j] = 0;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 3; j++)
		{
			c = deck[i * 3 + j];
			switch (c)
			{
			case 0: // Sebastian Moran
				tableCartes[i][7]++;
				tableCartes[i][2]++;
				break;
			case 1: // Irene Adler
				tableCartes[i][7]++;
				tableCartes[i][1]++;
				tableCartes[i][5]++;
				break;
			case 2: // Inspector Lestrade
				tableCartes[i][3]++;
				tableCartes[i][6]++;
				tableCartes[i][4]++;
				break;
			case 3: // Inspector Gregson
				tableCartes[i][3]++;
				tableCartes[i][2]++;
				tableCartes[i][4]++;
				break;
			case 4: // Inspector Baynes
				tableCartes[i][3]++;
				tableCartes[i][1]++;
				break;
			case 5: // Inspector Bradstreet
				tableCartes[i][3]++;
				tableCartes[i][2]++;
				break;
			case 6: // Inspector Hopkins
				tableCartes[i][3]++;
				tableCartes[i][0]++;
				tableCartes[i][6]++;
				break;
			case 7: // Sherlock Holmes
				tableCartes[i][0]++;
				tableCartes[i][1]++;
				tableCartes[i][2]++;
				break;
			case 8: // John Watson
				tableCartes[i][0]++;
				tableCartes[i][6]++;
				tableCartes[i][2]++;
				break;
			case 9: // Mycroft Holmes
				tableCartes[i][0]++;
				tableCartes[i][1]++;
				tableCartes[i][4]++;
				break;
			case 10: // Mrs. Hudson
				tableCartes[i][0]++;
				tableCartes[i][5]++;
				break;
			case 11: // Mary Morstan
				tableCartes[i][4]++;
				tableCartes[i][5]++;
				break;
			case 12: // James Moriarty
				tableCartes[i][7]++;
				tableCartes[i][1]++;
				break;
			}
		}
	}
}

void printDeck()
{
	int i, j;

	for (i = 0; i < 13; i++)
		printf("%d %s\n", deck[i], nomcartes[deck[i]]);

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 8; j++)
			printf("%2.2d ", tableCartes[i][j]);
		puts("");
	}
}

void printClients()
{
	int i;

	for (i = 0; i < nbClients; i++)
		printf("%d: %s %5.5d %s\n", i, tcpClients[i].ipAddress,
			   tcpClients[i].port,
			   tcpClients[i].name);
}

int findClientByName(char *name)
{
	int i;

	for (i = 0; i < nbClients; i++)
		if (strcmp(tcpClients[i].name, name) == 0)
			return i;
	return -1;
}

void sendMessageToClient(char *clientip, int clientport, char *mess)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(clientip);
	if (server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(clientport);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR connecting\n");
		exit(1);
	}

	sprintf(buffer, "%s\n", mess);
	write(sockfd, buffer, strlen(buffer));

	close(sockfd);
}

void broadcastMessage(char *mess)
{
	int i;

	for (i = 0; i < nbClients; i++)
		sendMessageToClient(tcpClients[i].ipAddress,
							tcpClients[i].port,
							mess);
}

// Pass to the next client that is not eliminated (!=1)
int joueursuivant(int joueurCourant, int *liste_joueurs_elimines)
{
	char buffer[256]; // for stocking messages
	if (all_users_are_eliminated(liste_joueurs_elimines))
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				sprintf(buffer, "V %d %d %d", i, j, tableCartes[i][j]);
				broadcastMessage(buffer);
			}
		}

		joueurCourant = 4;
	}
	else
	{
		joueurCourant++;
		if (joueurCourant > 3)
		{
			joueurCourant = 0;
		}

		while (liste_joueurs_elimines[joueurCourant] == 1)
		{
			joueurCourant++;
			if (joueurCourant > 3)
			{
				joueurCourant = 0;
			}
		}

		sprintf(buffer, "M %d", joueurCourant);
		broadcastMessage(buffer);
	}

	return joueurCourant;
}

int main(int argc, char *argv[])
{
	// CODE ADDED - Allows the cards to be get suffled
	srand(time(NULL)); // Seed the random number generator with current time

	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int i;

	char com;
	char clientIpAddress[256], clientName[256];
	int clientPort;
	int id;
	char reply[256];

	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *)&serv_addr,
			 sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	init_joueurs_elimines(); // CODE ADDED
	printDeck();
	melangerDeck();
	createTable();
	printDeck();
	joueurCourant = 0;

	for (i = 0; i < 4; i++)
	{
		strcpy(tcpClients[i].ipAddress, "localhost");
		tcpClients[i].port = -1;
		strcpy(tcpClients[i].name, "-");
	}

	while (1)
	{
		newsockfd = accept(sockfd,
						   (struct sockaddr *)&cli_addr,
						   &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");

		bzero(buffer, 256);
		n = read(newsockfd, buffer, 255);
		if (n < 0)
			error("ERROR reading from socket");

		printf("Received packet from %s:%d\nData: [%s]\n\n",
			   inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

		if (fsmServer == 0)
		{
			switch (buffer[0])
			{
			case 'C':
				sscanf(buffer, "%c %s %d %s", &com, clientIpAddress, &clientPort, clientName);
				printf("COM=%c ipAddress=%s port=%d name=%s\n", com, clientIpAddress, clientPort, clientName);

				// fsmServer==0 alors j'attends les connexions de tous les joueurs
				strcpy(tcpClients[nbClients].ipAddress, clientIpAddress);
				tcpClients[nbClients].port = clientPort;
				strcpy(tcpClients[nbClients].name, clientName);
				nbClients++;

				printClients();

				// rechercher l'id du joueur qui vient de se connecter

				id = findClientByName(clientName);
				printf("id=%d\n", id);

				// lui envoyer un message personnel pour lui communiquer son id

				sprintf(reply, "I %d", id);
				sendMessageToClient(tcpClients[id].ipAddress,
									tcpClients[id].port,
									reply);

				// Envoyer un message broadcast pour communiquer a tout le monde la liste des joueurs actuellement
				// connectes

				sprintf(reply, "L %s %s %s %s", tcpClients[0].name, tcpClients[1].name, tcpClients[2].name, tcpClients[3].name);
				broadcastMessage(reply);

				// Si le nombre de joueurs atteint 4, alors on peut lancer le jeu

				if (nbClients == 4)
				{
					// CODE ADDED HERE
					// We know that P1 takes 0,1,2, P2 takes, 3, 4, 5, P3 takes 6, 7, 8 and P4 takes 9, 10, 11
					int index = 0; // goes up to 11

					/* On envoie les cartes a chaque jouer */
					for (int local_clients = 0; local_clients < nbClients; local_clients++)
					{
						char *send_buffer = malloc(sizeof(char) * 256); // allocating 256 bytes
						sprintf(send_buffer, "D %d %d %d", deck[index], deck[index + 1], deck[index + 2]);
						sendMessageToClient(tcpClients[local_clients].ipAddress, tcpClients[local_clients].port, send_buffer);
						free(send_buffer); // freeing the allocated memory
						index += 3;		   // goes up to next client index in the list

						/* ainsi que la ligne qui lui correspond dans tableCartes */
						for (i = 0; i < 8; i++)
						{
							sprintf(reply, "V %d %d %d", local_clients, i, tableCartes[local_clients][i]);
							sendMessageToClient(tcpClients[local_clients].ipAddress, tcpClients[local_clients].port, reply); // this is sent to teh specific client
						}
					}

					// On envoie enfin un message a tout le monde pour definir qui est le joueur courant=0
					// CODE ADDED HERE
					sprintf(reply, "M %d", joueurCourant);
					broadcastMessage(reply); // this is sent to everyone (broadcast)

					fsmServer = 1;
				}
				break;
			}
		}
		else if (fsmServer == 1)
		{
			// SH13 game logic goes below. Every client can send "G %d %d" or "O %d %d" or "S %d %d %d"
			switch (buffer[0]) // login according to the first character of the message that is received
			{
			case 'G':
			{
				// Extracting the received infrmation
				int player_id;
				int guilt_guess;
				sscanf(buffer, "G %d %d", &player_id, &guilt_guess);

				// The coupable is the card 12.If the gues is the card 12, this player wins
				if (guilt_guess == deck[12])
				{
					// Generating infrmation message for the players
					char buffer[256];
					for (int i = 0; i < 4; i++)
					{
						for (int j = 0; j < 8; j++)
						{
							sprintf(buffer, "V %d %d %d", i, j, tableCartes[i][j]);
							broadcastMessage(buffer);

							if (player_id != -1)
							{
								if (i == player_id)
								{
									sprintf(buffer, "M %d", 5);
									sendMessageToClient(tcpClients[i].ipAddress, tcpClients[i].port, buffer);
								}
								else
								{
									sprintf(buffer, "M %d", 4);
									sendMessageToClient(tcpClients[i].ipAddress, tcpClients[i].port, buffer);
								}
							}
						}
					}

					sprintf(reply, ":D Le joueur %d a bien deviné, il a gagné :D \n", player_id);
					broadcastMessage(reply);
					return (0);
				}
				else // else this player loses
				{
					liste_joueurs_elimines[player_id] = 1; // setting up the user as eliminated
					sprintf(reply, "P %d %d %d %d", liste_joueurs_elimines[0], liste_joueurs_elimines[1], liste_joueurs_elimines[2], liste_joueurs_elimines[3]);
					broadcastMessage(reply);

					sprintf(reply, "M %d", 4);
					sendMessageToClient(tcpClients[player_id].ipAddress, tcpClients[player_id].port, reply);

					// Informting the world for the state of the user
					sprintf(reply, "Player P%d's guess was incorrect.\n", player_id + 1);
					broadcastMessage(reply);

					joueurCourant = joueursuivant(joueurCourant, liste_joueurs_elimines); // updating who is playing next

					if (all_users_are_eliminated(liste_joueurs_elimines)) // checking if all users are eliminated
					{
						sprintf(reply, "All the players are eliminated. You have lost !!");
						broadcastMessage(reply);
						return (0);
					}
				}
			}
			break;

			case 'O':
			{
				int player_id_O;
				int symbole_O;

				// Extracting the information
				sscanf(buffer, "O %d %d", &player_id_O, &symbole_O);

				// asking the question to all the other players
				for (int k = 0; k < 4; k++)
				{
					if (k == player_id_O)
					{
						continue; // this is us
					}
					else
					{
						for (int ki = 0; ki < 3; ki++)
						{
							if (tableCartes[k][symbole_O] != 0)
							{
								// The player ki has the symbole of the colonne colonne symbole_O
								sprintf(reply, "V %d %d %d", k, symbole_O, 100);
								broadcastMessage(reply);
							}

							else
							{
								sprintf(reply, "V %d %d %d", k, symbole_O, tableCartes[k][symbole_O]); // can de tableCartes[k][symbole_O] == 0
								broadcastMessage(reply);
							}
						}
					}
				}

				joueurCourant = joueursuivant(joueurCourant, liste_joueurs_elimines); // updating who is playing next
			}
			break;

			case 'S':
			{
				int askingId, askedId, askSymboleS;
				sscanf(buffer, "S %d %d %d", &askingId, &askedId, &askSymboleS);

				// Le joueur askedId a le symbole askSymbole - On l'envoie au joueur qui a demandé (askId)
				sprintf(reply, "V %d %d %d", askedId, askSymboleS, tableCartes[askedId][askSymboleS]);
				sendMessageToClient(tcpClients[askingId].ipAddress, tcpClients[askingId].port, reply);

				joueurCourant = joueursuivant(joueurCourant, liste_joueurs_elimines); // updating who is playing next
			}
			break;

			default:
				break;
			}
		}
		close(newsockfd);
	}
	close(sockfd);
	return 0;
}
