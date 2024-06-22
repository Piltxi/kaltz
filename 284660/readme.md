Per compilare:
    make
Oppure: 
    g++ -std=c++11 automaton.cpp

Per eseguire 
    ./automaton [namefile.txt]

Il programma genera le due rappresentazioni .png degli automi in lavorazione: NDFA.png e DFA.png
Per generare le rappresentazioni viene utilizzato il software Graphviz: se si è sprovvisti, commentare la riga #DEFINE DOT

è possibile specificare il nome del file di output con i dati richiesti del DFA; se questo non viene modificato, 
viene creato il file di testo "outputDFA.txt", formattato come richiesto. 

allego tre configurazioni su cui è stato testato il software:
    ./automaton input/input1.txt
    ./automaton input/input2.txt
    ./automaton input/input3.txt