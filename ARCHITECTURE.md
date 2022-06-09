
***************************************************
#Plan de travail

##Les tâches ont étées reparties de la façon suivante :

###Le premier rendu :
*Amira* :
  Faire les fonctions de la lecture de requête et l'écriture de la réponse du démon pour chaque option

*Aurélie*:
 fonctions auxiliaires pour les paths + la fonction qui cassini qui écriture la rêquete

*Benidy*:
fonctions auxiliaires pour le timing + ...
###Le second  rendu :

*Benidy*  : -c
        changement des writes succéssifs en écriture dans buffer (dans cassini).

*Aurélie* : -x/-q

FONCTION PRINCIPALE de saturnd qui selon l'opération envoyé par cassini efféctue le traitement

*Amira*   : -l/-r/-s/-o

==> après avoir réuni notre travail (pour chaque rendu), on a fait des réunions pour le débogage qu'on a efféctué sur une seule machine (soit celle de Aurélie, Amira ou Bénidy)
on a donc pu participer chacune à toutes les tâches et fonctions du projets.

#Architecture
Notre projet comporte un dossier src avec plusieurs fichier .c
####Il y a les deux fichiers sources principaux sont les suivant:

 `cassini.c` :

     cassini envoie la requête en mettant à la suite les différents arguments dans un buffer.
     Il traite ensuite la réponse de saturn en fonction de l'identifiant de la tâche et en  fonction des arguments passé en paramètre appelle la fonction approprié. La lecture de la réponse est déléguée à des fonctions auxiliaires qui récupèrent les données selon les cas.

`saturnd.c` :

    crée tout le serveur (client-démon)

`common.c`  :  

    contient les fonctions auxiliaires partagées entre cassini.c et saturnd.c

`list.c`    :

    contient la fonction qui traite l'argument -l et une fonction auxiliaire qui extrait le contenu d'un fichier

`create.c`  :

    contient la fonction qui traite l'argument -x et plusieurs fonctions auxiliaires

`remove.c`  :

    contient la fonction qui traite l'argument -r

`stdout_stderr.c`:  

    contient la fonction qui traite les deux arguments -o et -e

`terminate.c`:

    contient la fonction qui traite l'argument -q

`time_exitcode.c`:

    contient la fonction qui traite l'argument -x


####Il y a les deux fichiers headers sont les suivant:

`cassini.h`

    contient les inscludes + définitions de fonctions dans cassini.c

`commandline.h`

   définie une structure commandline

`common.h`  :

   contient les inscludes + définitions de fonctions dans common.c

`saturnd.h`

   contient les inscludes + définitions de fonctions dans saturnd.c

`saturnd_option.h`

   contient tous les includes que les fonctions d'options auront besoin + leurs définitions

`stringb.h`

   définie la structure string

`timing.h`

   définie la structure timing


## Remarques /Conclusion    

Nous avons pu employer tout ce que nous avons appris en C pendant ce semestre, le cours de Système d'exploitation ayant été
    d'une importance particulière. En effet, il nous a permis de comprendre les bases du travail en groupe avec les systèmes de contrôle de versions (git).

Dans l'ensemble, il y avait une bonne coordination dans les rôles de chacun et une bonne entente au sein de l'équipe était présente.
