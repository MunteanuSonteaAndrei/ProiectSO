Tool folosit: Gemini AI

1. Funcția `parse_condition`

Promptul oferit AI-ului:

Am nevoie de o funcție în C care parsează un string de forma "field:operator:value". Te rog să generezi funcția:
`int parse_condition(const char *input, char *field, char *op, char *value);`
Vreau să împarți string-ul folosind delimitatorul `:` cu ajutorul funcției `strtok`.

Ce a fost generat:
AI-ul a generat o funcție care apela direct `strtok(input, ":")` pentru a extrage cele 3 componente și apoi folosea `strcpy` pentru a le muta în variabilele destinație.

Ce am modificat și de ce:
Am observat o problemă: funcția `strtok` modifică șirul original (înlocuiește delimitatorii cu `\0`). Deoarece parametrul `input` din semnătura cerută este declarat ca fiind `const char *`, modificarea sa ar duce la un comportament nedefinit sau direct la o eroare de tip *Segmentation Fault* la execuție. 

Pentru a repara acest lucru, am adăugat un buffer temporar local:

char temp[256];
strncpy(temp, input, sizeof(temp));
temp[sizeof(temp) - 1] = '\0';

2 Funcția match_condition

Promptul oferit AI-ului:

Structura mea de date în C este următoarea:

typedef struct {
    int id;
    char inspector[32];
    float lat, lon;
    char category[32];
    int severity; 
    time_t timestamp;
    char description[128];
} Report;

Scrie funcția int match_condition(Report *r, const char *field, const char *op, const char *value); care returnează 1 dacă structura respectă condiția sau 0 dacă nu.

Ce a fost generat:
AI-ul a creat un lanț lung de instrucțiuni if / else if. Pentru câmpurile numerice, a folosit corect atoi(value). Totuși, la compararea câmpurilor de tip string (precum category sau inspector), AI-ul a scris cod de forma: if (r->category == value).

Ce am modificat și de ce:
În C, operatorul == aplicat pe două șiruri de caractere compară adresele lor de memorie (pointerii), nu conținutul lor textual. Codul generat ar fi returnat mereu 0.
Am modificat manual codul pentru a folosi funcția strcmp() deblocând astfel compararea corectă a șirurilor:

if (strcmp(op, "==") == 0) return strcmp(r->category, value) == 0;