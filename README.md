**Nume: Nicolae-Cristian Macovei**
**Grupă: 312CAb**

## Segregated Free Lists (SFL) - Tema 1

### Descriere:

Am implementat un alocator de memorie folosind structura de date `Segregated Free Lists`. Aceasta consta intr-un vector de liste dublu inlantuite circulare, fiecare continand blocuri de memorie nealocate de o anumita dimensiune. Blocurile alocate trebuie pastrate intr-o alta structura de date, pentru care eu am folosit un `ArrayList`.

### Comenzi: 

#### INIT_HEAP
```sh
INIT_HEAP <start_addr> <num_lists> <bytes_per_list> <reconstruction>
```
Comanda initializeaza heap-ul de memorie nealocata cu parametrii dati.
<br>
Prima adresa de la care se poate aloca memorie va fi `start_addr`.
<br>
Se creeaza `num_lists` liste cu blocuri de dimensiuni incepand de la 8, 16, ..., fiecare continand `bytes_per_list` bytes in total.
<br>
Daca `reconstruction` este 1, blocurile fragmentate vor fi reconstruite dupa eliberare.

#### MALLOC
```sh
MALLOC <num_bytes>
```
Comanda aloca un bloc de dimensiune `num_bytes`.
<br>
Pentru a aloca acest bloc, se cauta in SFL prima lista ce contine blocuri de dimensiune mai mare sau egala cu `num_bytes`. Din aceasta lista se ia primul bloc si daca e cazul se fragmenteaza intr-un bloc de `num_bytes` si un bloc de dimensiunea ramasa.
<br>
Set sterge apoi blocul din structura care contine blocuri nealocate (SFL) si se adauga un bloc nou in structura care contine blocurile alocate.
<br>
Blocul ramas in urma fragmentarii se muta din lista in care era intr-o noua lista, corespunzatoare dimensiunii lui.

#### FREE
```sh
FREE <addr>
```
Comanda elibereaza blocul de memorie alocat la adresa `addr`.
<br>
Fiecare bloc de memorie alocat are urmatoarele informatii: 
- adresa la care a fost alocat
- dimensiunea blocului

Se cauta blocul dupa adresa in structura care contine blocurile de memorie alocate.
Daca nu se gaseste, se va afisa mesajul `Invalid free.`
<br>
Dupa ce s-a gasit blocul, acesta se scoate din lista blocurilor alocate si se insereaza un bloc nou in structura de date ce contine blocurile nealocate.

#### Bonus - Reconstructie
In cadrul bonusului, trebuie facuta reconstructia unui bloc fragmentat.
<br>
Pentru asta, fiecare bloc (atat alocat, cat si nealocat) retine o informatie suplimentara, si anume `fragment_index`.
<br>
Astfel se alege un indice unic pentru fiecare fragmentare (initial este zero, apoi este incrementat la fiecare fragmentare).
De asemenea, la fiecare fragmentare se retin intr-o lista, pentru fiecare astfel de indice de fragmentare, urmatoarele informatii:
- `parent_fragm` - blocul care se fragmenteaza e posibil sa fi fost fragmentat in trecut, iar acea fragmentare trebuie stiuta pentru a putea repara recursiv
- dimensiunea si adresele celor doua blocuri rezultate in urma fragmentarii

Cand un bloc rezultat din urma fragmentarii este eliberat, inainte de a insera un nou bloc in lista de blocuri nealocate, se cauta intai un bloc cu indicele de fragmentare identic cu al blocului curent, iar daca se gaseste, cele doua blocuri se unesc intr-un bloc mai mare. Apoi, acest bloc e posibil sa fie la randul lui un fragment dintr-un bloc mai mare, deci `fragment_index`-ul noului bloc va fi `parent_fragm`-ul fragmentarii curente. Se incearca recursiv repararea acestei fragmentari, pana cand nu se gaseste cealalta jumatate sau se ajunge la `fragment_index` zero, ceea ce inseamna ca s-a reconstruit blocul original. Cand s-a ajuns aici, blocul se adauga in lista de blocuri libere.

#### DUMP_MEMORY
```sh
DUMP_MEMORY
```
Aceasta comanda afiseaza intr-un format human-readable atat layout-ul memoriei alocate si nealocate, cat si informatii precum numarul de apeluri malloc, numarul de apeluri free, numarul de blocuri fragmentate etc.

#### READ
```sh
READ <addr> <num_bytes>
```
Aceasta comanda citeste `num_bytes` din datele din memoria alocata la adresa `addr`.
<br>
Daca adresa `addr` nu reprezinta startul niciunui bloc de memorie alocat, se afiseaza mesajul `Segmentation Fault` si se ruleaza comanda `DUMP`.
<br>
Datele pot fi scrise pe mai multe blocuri, deci nu este suficienta verificarea daca adresa este alocata si blocul are dimensiunea suficienta. Se poate intampla ca blocul de la adresa `addr` sa aiba o dimensiune mai mica decat `num_bytes`, dar imediat dupa el sa urmeze alte blocuri, caz in care trebuie citite datele de pe mai multe blocuri.
<br>
Pentru ca citirea sa fie valida, toti bytes de la `addr` pana la `addr + num_bytes` sa fie intr-o zona de memorie alocata. Astfel am cautat blocul care incepe la adresa `addr`, dupa care am parcurs blocurile adiacente lui pana cand am trecut de adresa `addr + num_bytes` (caz in care afisarea e valida) sau pana cand zona alocata nu mai era contigua (caz in care afisarea nu e valida si se afiseaza seg fault).

#### WRITE
```sh
READ <addr> <string> <num_bytes>
```
Aceasta comanda scrie `num_bytes` dintr-un sir de caractere introdus de utilizator la adresa `addr` din memorie.
<br>
Verificarea daca adresa este alocata se face la fel ca in cazul comenzii `READ`.
<br>
O problema la aceasta comanda este ca utilizatorul poate introduce string-ul ca mai multe cuvinte separate prin spatii si delimitate de caracterele `"`. Din cauza asta, parsarea nu se poate realiza cu strtok, astfel ca am folosit o parsare speciala pentru aceasta comanda, parcurgand caracter cu caracter.

#### DESTROY_HEAP
```sh
DESTROY_HEAP
```
Aceasta comanda elibereaza toata memoria folosita de program si termina executia.
