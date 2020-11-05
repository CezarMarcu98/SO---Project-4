Marcu-Nicolescu Cezar-George
335CB

                        Tema 4 - Planificator de Threaduri


    In primul rand am inceput tema prin a ma informa de cum as putea sa implementez planificatorul Round-Robin.
Am descoperit ca mi-ar fi cel mai usor daca as folosi priority queuri pentru a o implementa . Planificatorul meu este format din o coada de prioritati pentru threadurile ce vor fi ready, o coada de prioritati ce contine toate threadurile (cele din new), un vector de structura special construita pentru cazul in care un thread devine blocant in urma unui apel de I/O, cat si cei 2 parametrii necesari unui planificator : numarul de dispozitive de I/O si cuanta de timp.
In so_init mi-am alocat elementele din structura planificatorului si am creat threadul principal.

In so_end , mai intai am scos toate threadurile din coada in care mi le stocam pentru a putea da join , finalizand orice actiune necesara. Am dezalocat tot ceea ce tine de Scheduler , priority queueul pentru threadurile din coada ready , cat si cele ce sunt lista de asteptare a fiecarui dispozitiv.

In so_fork , am creat threadul copil ce va urma a fi apelat cu create . Am initializat argumentele necesare pentru apelarea handlerului de catre threadul copil. A trebuit sa blochez executia segmentelor de cod in functie de thread cu ajutorul unei bariere. Dupa ce copilul a fost inserat in coada ready a planificatorului am putut anunta parintele ca isi poate continua executia , segmentul critic fiind depasit .
In structura de Threaduri , am 1 semafor . In momentul in care astept planificarea threadului pe scheduler este apelat wait-ul pe acest semafor si astept ca planificatorul sa incrementeze valoarea semaforului pentru a putea rula in continuare.
Asadar , in fuctia thread_go , noul thread rezultat din fork este instantiat cu valorile date ca parametru in functia fork, inserat in coada ready pentru planificare si pus in asteptare pentru a rula handlerul . Odata terminat , acesta este inlocuit cu alt thread si trecut in coada de threaduri (new).

Thredurile ce sunt puse in asteptare sunt inserate intr-o lista a dispozitivului ce a semnalat intreruperea. Asadar , in structura planificatorului , am un vector de structuri ce contin aceste liste.
In momentul in care este apelat un so_wait stiu direct in ce lista trebuie sa inserez.
So_exec-ul decrementeaza intotdeauna cuanta de timp si verifica daca threadul actual trebuie preemptat . In functia check_preempt fac acest lucru. Verific timpul de rulare al threadului actual . Daca i s-a scurs cuanta de timp , atunci ii verific prioritatea cu cea a primului thread din priorityQueue . Daca este mai mare cea a threadului actual , atunci poate ramane pe scheduler , iar cuanta sa de timp va fi reactualizata.
In cazul in care threadul actual are prioritatea egala cu a primului thread in asteptare pe planificator , acesta trebuie inlocuit.
Indiferent de cuanta de timp a threadului actual , daca un alt thread are o prioritate mai mare , automat este preemptat .
Daca threadul ce ruleaza acum intampina un apel blocant . Atunci trebuie scos de pe planificator si inlocuit. Aceasta schimbare o fac cu ajutorul unei variabile pe care o stochez in structura de threaduri . In momentul in care devine 1 , atunci stiu ca threadul curent este blocat in asteptarea unui I/O.
Semaforul threadului curent , daca este inlocuit de pe planificator este pus pe wait , pentru a-si continua executia atunci cand este reapelat. Semaforul threadului nou , ii va semnala acestuia ca ii incepe executia (cu un post), permitandu-i sa ruleze.

Atunci cand primesc un so_signal , pentru ca mi se da ca parametru dispozitivul apelant, atunci stiu in ce lista pot face modificari . Aceste modificari constau in scoaterea din lista a dispozitivelor ce sunt in asteptare si schimbarea acelui flag ce semnifica faptul ca threadul este blocat, dar si introducerea in coada ready a planificatorului.
In timp ce scot un thread din lista de asteptare , il inserez automat in priority queue-ul pentru ready threads, pentru a-si continua executia.

