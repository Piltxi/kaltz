Per compilare:
    make
Oppure: 
    g++ -std=c++11 ERtoND.cpp

Per eseguire 
    ./ernd [namefile.txt]

Il programma genera la rappresentazione .png dell'automa non deterministico derivato dalla espressione regolare: NDFA.png
Per generare le rappresentazioni viene utilizzato il software Graphviz: se si è sprovvisti, commentare la riga #DEFINE DOT

è possibile specificare il nome del file di output con i dati richiesti del NDFA; se questo non viene modificato, 
viene creato il file di testo "outNDFA.txt", formattato come richiesto. 

allego due configurazioni su cui è stato testato il software:
    ./ernd input/input01.txt
    ./ernd input/input02.txt