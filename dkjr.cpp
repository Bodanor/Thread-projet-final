#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE 0
#define DKJR 1
#define CROCO 2
#define CORBEAU 3
#define CLE 4

#define AUCUN_EVENEMENT 0

#define LIBRE_BAS 1
#define LIANE_BAS 2
#define DOUBLE_LIANE_BAS 3
#define LIBRE_HAUT 4
#define LIANE_HAUT 5

void *FctThreadEvenements(void *);
void *FctThreadCle(void *);
void *FctThreadDK(void *);
void *FctThreadDKJr(void *);
void *FctThreadScore(void *);
void *FctThreadEnnemis(void *);
void *FctThreadCorbeau(void *);
void *FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void initCle();
void DestructeurVS(void *p);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;
pthread_once_t controleur = PTHREAD_ONCE_INIT;

bool MAJDK = false;
int score = 0;
bool MAJScore = true;
int delaiEnnemis = 4000;
int positionDKJr = 1;
int evenement = AUCUN_EVENEMENT;
int etatDKJr;

typedef struct
{
	int type;
	pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
	bool haut;
	int position;
} S_CROCO;
int nbr_vies_perdus = 0;
// ------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	struct sigaction act;
	sigset_t mask;

	ouvrirFenetreGraphique();
	afficherCage(1);
	afficherCage(2);
	afficherCage(3);
	afficherCage(4);

	act.sa_flags = 0;
	act.sa_handler = HandlerSIGALRM;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, NULL);

	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	alarm(15);

	pthread_mutex_init(&mutexEvenement, NULL);
	pthread_mutex_init(&mutexDK, NULL);
	pthread_mutex_init(&mutexScore, NULL);
	pthread_cond_init(&condDK, NULL);
	pthread_cond_init(&condScore, NULL);

	if (pthread_key_create(&keySpec, DestructeurVS) != 0)
		perror("Impossible de cree la variable spécifique !\n");

	if (pthread_create(&threadCle, NULL, FctThreadCle, NULL) != 0)
		perror("Thread clé erreur !\n");

	if (pthread_create(&threadDK, NULL, FctThreadDK, NULL) != 0)
		perror("Thread DK erreur !\n");

	if (pthread_create(&threadEvenements, NULL, FctThreadEvenements, NULL) != 0)
		perror("Thread évenements erreur !\n");

	if (pthread_create(&threadScore, NULL, FctThreadScore, NULL) != 0)
		perror("Thread Score erreur !\n");

	if (pthread_create(&threadEnnemis, NULL, FctThreadEnnemis, NULL) != 0)
		perror("Thread Ennemis erreur !\n");

	while (nbr_vies_perdus != 3)
	{

		if (pthread_create(&threadDKJr, NULL, FctThreadDKJr, NULL) != 0)
			perror("Thread évenements erreur !\n");

		pthread_join(threadDKJr, NULL);

		if (nbr_vies_perdus != 0) // Eviter si on réussit du premier coup que l'affichage affiche un éche
			afficherEchec(nbr_vies_perdus);
	}

	pthread_join(threadEvenements, NULL);
}

// -------------------------------------

void initGrilleJeu()
{
	int i, j;

	pthread_mutex_lock(&mutexGrilleJeu);

	for (i = 0; i < 4; i++)
		for (j = 0; j < 7; j++)
			setGrilleJeu(i, j);

	pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
	grilleJeu[l][c].type = type;
	grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{
	for (int j = 0; j < 4; j++)
	{
		for (int k = 0; k < 8; k++)
			printf("%d  ", grilleJeu[j][k].type);
		printf("\n");
	}

	printf("\n");
}

void *FctThreadCle(void *p)
{
	struct timespec temps = {0, 700000000};

	while (nbr_vies_perdus != 3)
	{

		setGrilleJeu(0, 1, CLE);
		afficherGrilleJeu();
		afficherCle(1);
		nanosleep(&temps, NULL);
		effacerCarres(3, 11, 1, 2);
		effacerCarres(4, 12, 1, 1);

		/* Si catch de la cle */
		if (grilleJeu[0][1].type == DKJR)
		{
			effacerCarres(3, 11, 1, 4);
			nanosleep(&temps, NULL);
			nanosleep(&temps, NULL);
		}

		setGrilleJeu(0, 1, VIDE);
		afficherGrilleJeu();
		afficherCle(2);
		nanosleep(&temps, NULL);
		effacerCarres(3, 12, 2, 2);

		afficherGrilleJeu();
		afficherCle(3);
		nanosleep(&temps, NULL);
		effacerCarres(3, 13, 2, 2);

		afficherGrilleJeu();
		afficherCle(4);
		nanosleep(&temps, NULL);
		effacerCarres(2, 13, 3, 3);

		afficherGrilleJeu();
		afficherCle(3);
		nanosleep(&temps, NULL);
		effacerCarres(3, 13, 2, 2);

		afficherGrilleJeu();
		afficherCle(2);
		nanosleep(&temps, NULL);
		effacerCarres(3, 12, 2, 2);
	}
}
void *FctThreadEvenements(void *)
{
	struct timespec temps = {0, 100000000};
	int evt;
	while (1)
	{

		evt = lireEvenement();

		pthread_mutex_lock(&mutexEvenement);

		switch (evt)
		{
		case SDL_QUIT:
			exit(0);
			break;
		case SDLK_UP:
			printf("KEY_UP\n");
			evenement = SDLK_UP;
			break;
		case SDLK_DOWN:
			printf("KEY_DOWN\n");
			evenement = SDLK_DOWN;
			break;
		case SDLK_LEFT:
			printf("KEY_LEFT\n");
			evenement = SDLK_LEFT;
			break;
		case SDLK_RIGHT:
			evenement = SDLK_RIGHT;
			printf("KEY_RIGHT\n");
			break;
		default:
			evenement = AUCUN_EVENEMENT;
			break;
		}

		pthread_mutex_unlock(&mutexEvenement);
		if (pthread_kill(threadDKJr, SIGQUIT) != 0)
			perror("Erreur d'envoi du signal vers threadDKJr\n");
		nanosleep(&temps, NULL);
		evenement = AUCUN_EVENEMENT;
	}
}
void *FctThreadDKJr(void *p)
{
	struct sigaction act, act1, act2;
	struct timespec temps;
	sigset_t mask;
	/* Armement du signal sigquit */
	act.sa_flags = 0;
	act.sa_handler = HandlerSIGQUIT;
	sigemptyset(&act.sa_mask);
	sigaction(SIGQUIT, &act, NULL);

	/* Armement du signal sigquit */
	act1.sa_flags = 0;
	act1.sa_handler = HandlerSIGINT;
	sigemptyset(&act1.sa_mask);
	sigaction(SIGINT, &act1, NULL);

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);


	bool on = true;

	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3, 1, DKJR);
	afficherDKJr(11, 9, 1);
	etatDKJr = LIBRE_BAS;
	positionDKJr = 1;
	pthread_mutex_unlock(&mutexGrilleJeu);

	while (on)
	{
		pause();

		pthread_mutex_lock(&mutexEvenement);
		pthread_mutex_lock(&mutexGrilleJeu);
		switch (etatDKJr)
		{
		case LIBRE_BAS:
			switch (evenement)
			{
			case SDLK_LEFT:
				if (positionDKJr > 1)
				{
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					positionDKJr--;
					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7,
								 ((positionDKJr - 1) % 4) + 1);
				}
				break;
			case SDLK_RIGHT:

				if (positionDKJr < 7)
				{
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					positionDKJr++;
					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
				}
				break;
			case SDLK_UP:

				if (grilleJeu[2][positionDKJr].type == CORBEAU)
				{
					pthread_kill(grilleJeu[2][positionDKJr].tid, SIGUSR1);
					nbr_vies_perdus++;
					effacerCarres(9, (positionDKJr * 2) + 7, 4, 4);
					setGrilleJeu(3, positionDKJr);
					on = 0;
				}
				else if (positionDKJr == 1 || positionDKJr == 5)
				{
					setGrilleJeu(3, positionDKJr);
					etatDKJr = LIANE_BAS;
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					afficherDKJr(10, (positionDKJr * 2) + 7, 7);
					setGrilleJeu(2, positionDKJr, DKJR);
				}
				else if (positionDKJr == 7)
				{
					setGrilleJeu(3, positionDKJr);
					etatDKJr = DOUBLE_LIANE_BAS;
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					afficherDKJr(10, (positionDKJr * 2) + 7, 5);
					setGrilleJeu(2, positionDKJr, DKJR);
				}
				else
				{
					etatDKJr = LIBRE_HAUT;
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2); // Effacer image actuelle de dkjr
					afficherDKJr(10, (positionDKJr * 2) + 7, 8);
					setGrilleJeu(2, positionDKJr, DKJR);
					temps.tv_sec = 1;
					temps.tv_nsec = 400000000;
					nanosleep(&temps, NULL);

					if (grilleJeu[3][positionDKJr].type == CROCO)
					{
						printf("Position dkjr = %d\n", (positionDKJr * 2) + 7);
						pthread_kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);
						nbr_vies_perdus++;
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(3, positionDKJr);
						on = 0;

					}
					else {
						setGrilleJeu(2, positionDKJr);
						setGrilleJeu(3, positionDKJr, DKJR);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2); // Effacer image actuelle de dkjr
						afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_BAS;
					}
				}
				break;
			}
			break;

		case LIANE_BAS:
			switch (evenement)
			{
			case SDLK_DOWN:
				etatDKJr = LIBRE_BAS;
				setGrilleJeu(2, positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
				setGrilleJeu(3, positionDKJr, DKJR);
				afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);

				break;
			}
			break;

		case DOUBLE_LIANE_BAS:
			switch (evenement)
			{
			case SDLK_DOWN:

				setGrilleJeu(2, positionDKJr);
				etatDKJr = LIBRE_BAS;
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
				setGrilleJeu(3, positionDKJr, DKJR);
				afficherDKJr(11, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);

				break;
			case SDLK_UP:

				etatDKJr = LIBRE_HAUT;
				setGrilleJeu(2, positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
				afficherDKJr(9, (positionDKJr * 2) + 7, 6);
				setGrilleJeu(1, positionDKJr, DKJR);
				break;
			}
			break;
		case LIBRE_HAUT:
			switch (evenement)
			{
			case SDLK_UP:

				if (positionDKJr < 7)
				{
					if (positionDKJr == 6)
					{
						setGrilleJeu(1, positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
						afficherDKJr(6, (positionDKJr * 2) + 7, 7);
						setGrilleJeu(0, positionDKJr, DKJR);
						etatDKJr = LIANE_HAUT;
					}
					else
					{
						setGrilleJeu(1, positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2); // Effacer image actuelle de dkjr
						afficherDKJr(6, (positionDKJr * 2) + 7, 8);
						setGrilleJeu(0, positionDKJr, DKJR);
						temps.tv_sec = 1;
						temps.tv_nsec = 400000000;
						nanosleep(&temps, NULL);

						setGrilleJeu(0, positionDKJr);
						effacerCarres(6, (positionDKJr * 2) + 7, 2, 2); // Effacer image actuelle de dkjr
						afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
						setGrilleJeu(1, positionDKJr, DKJR);
					}
				}
				break;
			case SDLK_DOWN:
				if (positionDKJr == 7)
				{
					setGrilleJeu(3, positionDKJr);
					effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					afficherDKJr(10, (positionDKJr * 2) + 7, 5);
					setGrilleJeu(2, positionDKJr, DKJR);
					etatDKJr = DOUBLE_LIANE_BAS;
				}
				break;
			case SDLK_LEFT:

				if (positionDKJr > 3)
				{
					setGrilleJeu(1, positionDKJr);
					effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					positionDKJr--;
					setGrilleJeu(1, positionDKJr, DKJR);
					afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
				}
				else
				{
					temps.tv_sec = 0;
					temps.tv_nsec = 700000000;
					effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
					afficherDKJr(6, (positionDKJr * 2) + 7, 9);
					setGrilleJeu(0, positionDKJr, DKJR);

					/* Si il attrape la clé */
					if (grilleJeu[0][1].type == CLE)
					{
						setGrilleJeu(0, 1, DKJR);
						nanosleep(&temps, NULL);

						positionDKJr--;
						effacerCarres(5, (positionDKJr * 2) + 8, 3, 2);
						afficherDKJr(6, (positionDKJr * 2) + 7, 10);
						nanosleep(&temps, NULL);

						effacerCarres(3, 11, 3, 3);
						afficherCage(4); // Réafficher la cage lorsque dkjr attrape la clef

						pthread_mutex_lock(&mutexDK);
						MAJDK = true;
						pthread_mutex_unlock(&mutexDK);
						pthread_cond_signal(&condDK);

						/* Augmenter le score */
						pthread_mutex_lock(&mutexScore);
						score += 10;
						MAJScore = false;
						pthread_mutex_unlock(&mutexScore);
						pthread_cond_signal(&condScore);
					}
					else
					{
						nanosleep(&temps, NULL);
						positionDKJr--;
						effacerCarres(5, (positionDKJr * 2) + 8, 3, 2);
						afficherDKJr(7, (positionDKJr * 2) + 7, 12);
						nanosleep(&temps, NULL);

						effacerCarres(6, (positionDKJr * 2) + 7, 3, 2);
						afficherDKJr(11, 7, 13);

						nanosleep(&temps, NULL); // Le temps que Dkjr renaisse de ses cendres

						effacerCarres(11, 7, 2, 2); // Effacement du dkjr qui se crash dans le buisson

						nbr_vies_perdus++;
					}
					on = 0;
					setGrilleJeu(0, positionDKJr + 1);
				}
				break;
			case SDLK_RIGHT:
				if (positionDKJr < 7)
				{
					if (positionDKJr == 6)
					{

						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
						positionDKJr++;
						afficherDKJr(7, (positionDKJr * 2) + 7, 6);
					}

					else
					{
						setGrilleJeu(1, positionDKJr);
						effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
						positionDKJr++;
						setGrilleJeu(1, positionDKJr, DKJR);
						afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
					}
				}
				break;
			}
			break;
		case LIANE_HAUT:
			switch (evenement)
			{
			case SDLK_DOWN:
				setGrilleJeu(0, positionDKJr);
				etatDKJr = LIBRE_HAUT;
				effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
				setGrilleJeu(1, positionDKJr, DKJR);
				afficherDKJr(7, (positionDKJr * 2) + 7, ((positionDKJr - 1) % 4) + 1);
				break;
			}
			break;
		}
		pthread_mutex_unlock(&mutexGrilleJeu);
		pthread_mutex_unlock(&mutexEvenement);
	}
	pthread_exit(0);
}
void *FctThreadDK(void *p)
{
	struct timespec temps = {0, 700000000};

	while (nbr_vies_perdus != 3)
	{
		pthread_mutex_lock(&mutexDK);
		while (!MAJDK)
			pthread_cond_wait(&condDK, &mutexDK);
		MAJDK = false;
		pthread_mutex_unlock(&mutexDK);
		effacerCarres(2, 7, 2, 2);

		pthread_mutex_lock(&mutexDK);
		while (!MAJDK)
			pthread_cond_wait(&condDK, &mutexDK);
		MAJDK = false;
		pthread_mutex_unlock(&mutexDK);
		effacerCarres(2, 9, 2, 2);

		pthread_mutex_lock(&mutexDK);
		while (!MAJDK)
			pthread_cond_wait(&condDK, &mutexDK);
		MAJDK = false;
		pthread_mutex_unlock(&mutexDK);
		effacerCarres(4, 7, 2, 2);

		pthread_mutex_lock(&mutexDK);
		while (!MAJDK)
			pthread_cond_wait(&condDK, &mutexDK);
		MAJDK = false;
		pthread_mutex_unlock(&mutexDK);
		effacerCarres(4, 9, 2, 3);

		/* Augmenter le score */
		pthread_mutex_lock(&mutexScore);
		score += 10;
		MAJScore = false;
		pthread_mutex_unlock(&mutexScore);
		pthread_cond_signal(&condScore);

		afficherRireDK();
		nanosleep(&temps, NULL);
		MAJDK = false;

		effacerCarres(3, 8, 2, 2); // Supprimer le rire
		afficherCage(1);
		afficherCage(2);
		afficherCage(3);
		afficherCage(4);
	}
}
void *FctThreadScore(void *p)
{
	afficherScore(score);
	while (nbr_vies_perdus != 3)
	{
		pthread_mutex_lock(&mutexScore);
		while (MAJScore)
			pthread_cond_wait(&condScore, &mutexScore);

		afficherScore(score);
		MAJScore = true;
		pthread_mutex_unlock(&mutexScore);
	}
}
void *FctThreadEnnemis(void *)
{
	pthread_t ThreadCorbeau, ThreadCroco;
	sigset_t mask;
	srand(time(NULL));
	int type_enemis;
	struct timespec temps = {0, 0};

	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	while (nbr_vies_perdus != 3)
	{
		type_enemis = 1; // rand() % 2;
		if (type_enemis == 0)
		{
			if (pthread_create(&ThreadCorbeau, NULL, FctThreadCorbeau, NULL) != 0)
				perror("Thread corbeau erreur !\n");
			printf("Thread corbeau crée ! \n");
		}
		else
		{
			if (pthread_create(&ThreadCroco, NULL, FctThreadCroco, NULL) != 0)
				perror("Thread croco erreur !\n");
			printf("Thread croco crée ! \n");
		}
		temps.tv_sec = delaiEnnemis / 1000;
		temps.tv_nsec = (delaiEnnemis % 1000) * 1000000;

		nanosleep(&temps, NULL);

		pthread_join(ThreadCroco, NULL);
	}
}
void *FctThreadCorbeau(void *p)
{
	sigset_t mask;
	struct timespec temps = {0, 700000000};
	int position = 8;

	int *pSpec; // Variable stockant la position horizontale sur la grille.

	struct sigaction act;

	/* Armement du signal sigquit */
	act.sa_flags = 0;
	act.sa_handler = HandlerSIGUSR1;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	pthread_once(&controleur, initCle);
	if ((pSpec = (int *)pthread_getspecific(keySpec)) == NULL)
	{

		pSpec = (int *)malloc(sizeof(int));
		pthread_setspecific(keySpec, pSpec);
		*pSpec = 0;
	}

	while (position <= 22)
	{
		if (grilleJeu[2][*pSpec].type == DKJR)
		{
			pthread_kill(threadDKJr, SIGINT);
			pthread_exit(0);
		}

		afficherCorbeau(position, (*pSpec % 2) + 1);
		setGrilleJeu(2, *pSpec, CORBEAU, pthread_self());

		if (pthread_setspecific(keySpec, pSpec))
			perror("Erreur de setspecific\n");

		position = position + 2;
		(*pSpec)++;

		nanosleep(&temps, NULL);

		setGrilleJeu(2, *pSpec - 1);
		effacerCarres(9, position - 2, 2, 2);
	}
}
void *FctThreadCroco(void *p)
{
	sigset_t mask;
	struct timespec temps = {0, 700000000};

	S_CROCO *pSpec; // Variable stockant la position horizontale sur la grille.

	struct sigaction act;

	/* Armement du signal sigquit */
	act.sa_flags = 0;
	act.sa_handler = HandlerSIGUSR2;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR2, &act, NULL);

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	
	pthread_once(&controleur, initCle);
	if ((pSpec = (S_CROCO *)pthread_getspecific(keySpec)) == NULL)
	{
		pSpec = (S_CROCO *)malloc(sizeof(S_CROCO));
		pthread_setspecific(keySpec, pSpec);
		pSpec->haut = 1;
		pSpec->position = 2;
	}

	while (pSpec->position != 1)
	{
		
		if (pSpec->haut == 1)
		{
			if (pSpec->position == 8)
			{
				afficherCroco(pSpec->position, 3);
				pSpec->position += 1;
			}
			else
			{
				afficherCroco(pSpec->position * 2 + 7, (pSpec->position % 2) + 1);
				setGrilleJeu(1, pSpec->position, CROCO, pthread_self());
				pSpec->position += 1;
			}
		}
		else
		{
			pSpec->position -= 1;
			afficherCroco((pSpec->position * 2) + 8, (pSpec->position % 2) + 4);
			setGrilleJeu(3, pSpec->position, CROCO, pthread_self());
		}
		nanosleep(&temps, NULL);

		/* Effacement du précedent crocro */
		if (pSpec->haut == 1)
		{
			if (pSpec->position > 8)
			{
				pSpec->position -= 1;
				effacerCarres(9, 23, 1, 1);
				pSpec->haut = 0;
			}
			else
			{
				effacerCarres(8, (pSpec->position * 2) + 5, 1, 1);
				setGrilleJeu(1, pSpec->position - 1);
			}
		}
		else
		{
			effacerCarres(12, (pSpec->position * 2) + 8, 1, 1);
			setGrilleJeu(3, pSpec->position);
		}
	}
}

void HandlerSIGQUIT(int sig)
{
	printf("SIGQUIT pour le thread (%u)\n", (unsigned int)pthread_self());
	fflush(stdout);
}
void HandlerSIGALRM(int sig)
{
	if (delaiEnnemis != 2500)
	{
		delaiEnnemis = delaiEnnemis - 250;
		alarm(15);
	}
}
void HandlerSIGUSR1(int sig)
{
	long varspec = (long)pthread_getspecific(keySpec); // Pour la compilation

	printf("SIGUSR1 pour le thread %u (%ld)\n", (unsigned int)pthread_self(), varspec);
	setGrilleJeu(2, varspec);
	effacerCarres(9, varspec * 2 + 7, 2, 2);
	pthread_exit(0);
}
void HandlerSIGUSR2(int sig)
{
	S_CROCO *varspec = (S_CROCO*)pthread_getspecific(keySpec); // Pour la compilation

	printf("SIGUSR2 pour le thread %u (%ld)\n", (unsigned int)pthread_self(), varspec->position);
	setGrilleJeu(3, varspec->position);
	effacerCarres(12, varspec->position * 2 + 8, 1, 1);
	pthread_exit(0);
}
void HandlerSIGINT(int sig)
{
	effacerCarres(9, (positionDKJr * 2) + 7, 4, 4);
	setGrilleJeu(2, positionDKJr);
	pthread_mutex_unlock(&mutexEvenement);
	pthread_mutex_unlock(&mutexGrilleJeu);
	nbr_vies_perdus++;

	pthread_exit(0);
}
void DestructeurVS(void *p)
{
	printf("Destruction de la variable specifique\n");
	free(p);
}
void initCle()
{
	printf("Initialisation de la clé \n");
	pthread_key_create(&keySpec, DestructeurVS);
}