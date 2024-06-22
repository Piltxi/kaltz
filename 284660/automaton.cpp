/**
 * @file automaton.cpp
 * @author Elia Pitzalis [284660@studenti.unimore.it]
 * @brief library of functions for the analysis of finite automata; includes subset construction algorithm and image generation (via dot) of the automata
 * @version 0.1
 * @date 2023-10-27
 * 
 * @note g++ -std=c++11 to compile
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <cctype> 
#include <list>
#include <queue>
#include <sstream>
#include <vector>

// Comment if you don't want to generate png of the automaton
#define DOT 

// Some debug print
// #define DEBUG 

const char epsilon = '*'; // '*' simulates the Greek epsilon

using namespace std; 

template <typename State, typename Symbol, typename TransitionType>
class Automaton {

    private: 
        State initialState; 
        State finalStates; 
        set<Symbol> alphabet; 
        TransitionType transitions; 
        vector<State> allStates; 

    public: 
        Automaton () {}

    void setInitialState(const State& initialState) {
        this->initialState = initialState;
    }

    void setFinalStates(const State& finalStates) {
        this->finalStates = finalStates;
    }

    void setAlphabet(const set<Symbol>& alphabet) {
        this->alphabet = alphabet;
    }

    void setTransitions (const TransitionType& transitions) {
        this->transitions = transitions; 
    }

    const TransitionType& getTransitions() const {
        return transitions;
    }

    const State& getStartState () const {
        return initialState; 
    }

    const State& getFinalStates() const {
        return finalStates;
    }
    
    const set<Symbol>& getAlphabet() const {
        return alphabet; 
    }

    void addState(const State& state) {
        if (find(allStates.begin(), allStates.end(), state) == allStates.end())
            allStates.push_back(state);
    }
    
    const vector<State>& getAllStates () const {
        return allStates;
    }

    void printAll () const {
        // default automaton print
    }   

    /**
     * @brief Get the State By Index object
     * 
     * @param index 
     * @return const set<int>& returns the State stored at a given index (position)
     */

    const set<int>& getStateByIndex(int index) const {
        if (index >= 0 && index < allStates.size()) {
            auto it = allStates.begin();
            advance(it, index);
            return *it;
        } 
        else
            throw out_of_range("State index is out of range.");
    }

    /**
     * @brief number of states stored in the automaton object
     * 
     * @return int size of vector<State>
     */
    int countAllStates() const {
        return allStates.size();
    }
}; 

/**
 * @brief Customized printing method for non-deterministic finite automata (DFA).
 * 
 * This specialization of the printAll function is used to print detailed information about a non-deterministic finite automaton (NDFA).
 * It includes the alphabet, the number of states, the start state, the set of states, transition details, and final state.
 * 
 * @note This specialized printing is tailored for NDFA objects with specific template parameters.
 */
template <>
void Automaton<int, char, multimap<pair<int, char>, int>>::printAll() const {

    cout<<"\nnondeterministic finite automaton data:\n"; 
    cout << "Alphabet:";
    for (const char& symbol : alphabet)
        cout << " '" << symbol << "'";
    cout<<endl; 

    cout<<"Number of States: "<<(getAllStates().size())<<"\tStart State: "<<getStartState()<<endl;

    cout<<"States in order of visit: ";
    for (const int& state : getAllStates())
        cout<<state<<" "; 
    cout<<endl; 

    pair<int, char> currentKey;
    multimap< pair<int, char>, int> transitions = getTransitions(); 

    cout << "\nTransitions:" <<  endl;
    for (const auto& entry : transitions) {
        if (entry.first != currentKey) {
            (currentKey != pair <int, char>() ? cout<<endl : cout<<"" );
            entry.first.second=='*' ? cout<< "fromState: " << entry.first.first <<" >"<<"\u03B5"<<endl : cout<< "fromState: " << entry.first.first <<" >"<<entry.first.second<<endl ;
            currentKey = entry.first;
        }
        cout << "\t-> " << entry.second << endl;
    }

    cout << "\nFinal State: " << getFinalStates()<<endl; 
    
    cout << endl; 
}

/**
 * @brief Customized printing method for deterministic finite automata (DFA).
 * 
 * This specialization of the printAll function is used to print detailed information about a deterministic finite automaton (DFA) after subset construction.
 * It includes the alphabet, the number of states, the start state, the set of states, transition details, and final state(s).
 * 
 * @note This specialized printing is tailored for DFA objects with specific template parameters.
 */
template <>
void Automaton<set<int>, char, map<pair<set<int>, char>, set<int>>>::printAll() const{

    cout<<"\ndeterministic finite automaton data after subset construction:\n"; 
    cout << "Alphabet:";
    for (const char& symbol : alphabet)
        cout << " '" << symbol << "'";
    cout<<endl; 

    cout<<"Number of States: "<<(getAllStates().size()); 
    cout<< "\tStart State: { ";
    for (const int& state: getStartState()) 
        cout<<state << " ";
    cout << "} "<< endl; 

    int index = 0; 
    cout<<"States: \n";
    for (const set<int>& subState : getAllStates()) {
        cout<<index<<"] "; 
        for (int value : subState) {
            cout << value << " ";
        }
        index++; cout << endl;
    }
    cout<<endl; 

   cout << "Transitions:" << endl;
    
    const auto& transitions = getTransitions();
    
    for (const auto& entry : transitions) {
        const set<int>& fromState = entry.first.first;
        const char symbol = entry.first.second;
        const set<int>& toState = entry.second;

        cout << "fromState: {";
        bool firstState = true;
        for (const int& state : fromState) {
            if (!firstState) {
                cout << ", ";
            }
            cout << state;
            firstState = false;
        }
        cout << "} > " << symbol << endl;

        cout << "\t-> {";
        bool firstToState = true;
        for (const int& state : toState) {
            if (!firstToState) {
                cout << ", ";
            }
            cout << state;
            firstToState = false;
        }
        cout << "}" << endl;
    }

    cout << "\nIndex of final State: { ";
    for (const int& state: getFinalStates()) cout<<state << " ";
    cout << "} "<< endl; 

    for (const int& elements : getFinalStates()){
        cout<<elements<<"] { "; 
        for (const int& state: getStateByIndex(elements))
            cout<<state<<" "; 
        cout<<"}"<<endl; 
    }

    cout << endl; 
}

/**
 * @brief checks character validity to represent new state
    To be implemented when changing the type of acceptable values
 * 
 * @param character 
 * @return true accepted character
 * @return false rejected character
 */
bool isState (const char character) {
    
    #ifdef DEBUG
        cout << "STATE -> check on " << character << ".\n"; 
    #endif

    if (isdigit(character))
        return true; 
    
    return false; 
}

/**
 * @brief checks character validity to represent new symbol
    To be implemented when changing the type of acceptable values
 * 
 * @param character 
 * @return true accepted character
 * @return false rejected character
 */
bool isSymbol (const char character) {
    
    #ifdef DEBUG
        cout << "SYMBOL -> check on " << character << ".\n"; 
    #endif

    if (isalpha(character) || isdigit(character))
        return true; 

    return false; 
}

/**
 * @brief Find the index of a state in the deterministic automaton (DFA).
 * 
 * This function searches for a specific state within the list of all states in a deterministic automaton (DFA).
 * If the state is found, its index in the list is returned. If the state is not found, -1 is returned, indicating that
 * the state is not present in the automaton.
 * 
 * @param dfa The deterministic automaton (DFA) to search within.
 * @param state The state for which to find the index.
 * @return int The index of the state if found, or -1 if the state is not present.
 */
int getIndexForState(const Automaton<set<int>, char, map<pair<set<int>, char>, set<int>>>& dfa, const set<int>& state) {
    int index = 0;
    for (const set<int>& subState : dfa.getAllStates()) {
        if (subState == state) {
            return index;
        }
        index++;
    }
    return -1;
}

/**
 * @brief Get the value at a specified index from a set.
 * This function utilizes generics to retrieve a value from a set.
 *
 * @tparam T The type of elements stored in the set.
 * @param set The set from which to retrieve the value.
 * @param index The index of the value to be retrieved.
 * @return T The value at the specified index in the set.
 * @throws out_of_range if the index is out of range for the set.
 */
template <typename T>
T getValueAtIndex(const set<T>& set, size_t index) {
    
    if (index > 0 && index <= set.size()) {
        auto it = set.begin();
        advance(it, index-1);
        return *it;
    } 
    else
        throw out_of_range("Index out of range");
}

/**
 * @brief Find and return the start state of a finite automaton by analyzing its transitions.
 * 
 * @param transitions A multimap representing transitions in the automaton.
 * @return int The start state if found; -1 if not found.
* @note tailored for NDFA objects with specific template parameters.
 */
int findStartState(const multimap<pair<int, char>, int>& transitions) {
    set<int> stateStart;
    set<int> stateEnd;

    for (const auto& entry : transitions) {
        stateStart.insert(entry.first.first);
        stateEnd.insert(entry.second);
    }

    for (int value : stateStart)
        if (stateEnd.find(value) == stateEnd.end())
            return value;
    return -1; 
}

/**
 * @brief Import a Non-Deterministic Finite Automaton (NDFA) from a file and construct an Automaton object
 * 
 * @param nameFile The name of the file containing NDFA description
 * @return Automaton<int, char, multimap<pair<int, char>, int>> The constructed NDFA object.
 */
Automaton<int, char, multimap<pair<int, char>, int>> importNDFA (const char* nameFile) {

    int stateInReading = 0; //debug print

    Automaton<int, char, multimap<pair<int, char>, int>> automaton;
    
    set<char> alphabet;
    int finalState;
    multimap<pair<int, char>, int> transitions; 
    
    alphabet.insert(epsilon); 

    ifstream fi (nameFile);
    if (fi.is_open()) {
        
        // Alphabet acquisition
        string line; 
        getline(fi, line); 

        for (const char& character : line) 
            if (isSymbol(character))
               alphabet.insert(character); 
        
        automaton.setAlphabet(alphabet); 

        // Final state acquisition
        fi >> finalState; 
        fi.ignore(numeric_limits<streamsize>::max(), '\n'); //Skip line
        automaton.setFinalStates(finalState); 

        #ifdef DEBUG 

            cout << "Alphabet Lenght: |" << alphabet.size() <<"|\t";
            for (const char& symbol : alphabet)
                cout << " '" << symbol << "'";
            
            cout<<endl; 
        
            cout<< "Final State recognized: "<< finalState<<endl; 

            cout<<endl; 

        #endif     

        while (!fi.eof()) {

            for (int i=1; i<=alphabet.size(); i++) {
                
                int toState;
                string line;

                // Leggi la riga corrente
                getline(fi, line);

                // Controlla se la riga è vuota
                if (line.empty()) {

                    continue; // Salta il giro del ciclo se la riga è vuota
                }
                char inputAlpha = getValueAtIndex(alphabet, i);
                istringstream lineStream(line);
                while (lineStream >> toState) {
                    transitions.emplace(make_pair(make_pair(stateInReading, inputAlpha), toState));
                }
            }
            stateInReading ++; 
        }
    }

    else {
        cout<<"\n\nERROR - Opening File - ImportNDFA\n\n";
        return automaton; 
    }
    
    automaton.setTransitions(transitions); 

    stateInReading --; 

    #ifdef DEBUG 
        cout<<"Numero di stati riconosciuti: "<<stateInReading<<endl; 
    #endif
    fi.close();     
    
    automaton.setInitialState(findStartState(automaton.getTransitions())); 

    for (const auto& entry : automaton.getTransitions()) {
        automaton.addState(entry.first.first); 
        automaton.addState(entry.second);
    }

    return automaton; 
}
/**
 * @brief Calculates the epsilon closure of states in an NDFA
 * @param NDFA non-deterministic finite automaton
 * @param states state to calculate epsilon closure
 * @return epsilon closure of set
 */
set<int> epsilonClosure(const Automaton<int, char, multimap<pair<int, char>, int>>& NDFA, const set<int>& states) {
    
    set<int> closure = states;
    queue<int> stateQueue;

    for (const int& state : states)
        stateQueue.push(state);

    while (!stateQueue.empty()) {
        int current = stateQueue.front();
        stateQueue.pop();

        for (const auto& entry : NDFA.getTransitions()) {
            if (entry.first.first == current && entry.first.second == epsilon) {
                
                int nextState = entry.second;
                
                if (closure.find(nextState) == closure.end()) {
                    closure.insert(nextState);
                    stateQueue.push(nextState);
                }
            }
        }
    }
    return closure;
}

/**
 * @brief Converts a non-deterministic finite automaton (NDFA) to a deterministic finite automaton (DFA).
 * ... using Subset Construction algorithm (it calculate in set<int> epsilonClosure set of corresponding states)
 * @param NDFA non-deterministic finite automaton to be converted
 * @return deterministic finite automaton
 */
Automaton<set<int>, char, map<pair<set<int>, char>, set<int>>> NDtoD(const Automaton<int, char, multimap<pair<int, char>, int>>& NDFA) {
    Automaton<set<int>, char, map<pair<set<int>, char>, set<int>>> DFA;

    set<char> alphabet = NDFA.getAlphabet();
    alphabet.erase(epsilon);
    DFA.setAlphabet(alphabet);

    queue<set<int>> stateQueue;
    set<set<int>> visitedStates;

    set<int> initialStateSet;
    initialStateSet.insert(NDFA.getStartState());
    set<int> zeroEpsilonClosure = epsilonClosure(NDFA, initialStateSet);
    DFA.setInitialState(zeroEpsilonClosure);

    stateQueue.push(zeroEpsilonClosure);
    visitedStates.insert(zeroEpsilonClosure);
    
    DFA.addState(zeroEpsilonClosure); 

    map<pair<set<int>, char>, set<int>> transitions;

    while (!stateQueue.empty()) {
        set<int> currentStateSet = stateQueue.front();
        stateQueue.pop();

        for (char symbol : alphabet) {
            set<int> nextStateSet;

            for (const int& state : currentStateSet) {
                multimap<pair<int, char>, int> nfaTransitions = NDFA.getTransitions();
                auto range = nfaTransitions.equal_range(make_pair(state, symbol));

                for (auto it = range.first; it != range.second; ++it) {
                    set<int> epsilonClosureSet = epsilonClosure(NDFA, {it->second});
                    nextStateSet.insert(epsilonClosureSet.begin(), epsilonClosureSet.end());
                }
            }

            set<int> epsilonClosureSet = epsilonClosure(NDFA, nextStateSet);

            if (!epsilonClosureSet.empty()) {
                transitions.emplace(make_pair(make_pair(currentStateSet, symbol), epsilonClosureSet));

                if (visitedStates.find(epsilonClosureSet) == visitedStates.end()) {
                    stateQueue.push(epsilonClosureSet);
                    visitedStates.insert(epsilonClosureSet);
                    DFA.addState(epsilonClosureSet); 
                }
            }
        }
    }

    DFA.setTransitions(transitions);

    int nfaFinalState = NDFA.getFinalStates();

    set<int> finalStatesDFAIndices;
    vector<set<int>> work = DFA.getAllStates(); 
    for (int i = 0; i < work.size(); ++i) {
        const set<int>& stateSet = work[i];
        if (stateSet.find(nfaFinalState) != stateSet.end())
            finalStatesDFAIndices.insert(i);
    }

    DFA.setFinalStates(finalStatesDFAIndices);

    return DFA;
}

/**
 * @brief Writes the DFA (Deterministic Finite Automaton) represented by the given automaton to a file.
 *
 * This function writes the state information, final states, and transitions of a DFA to a text file.
 * The data is formatted according to the homework standard requirements.
 * 
 * @param automaton The DFA to be written to the file.
 * @param nameFile The name of the output file where the DFA will be saved.
 */
template <typename State, typename Symbol, typename TransitionType>
void writeOutputDFA (const Automaton<State, Symbol, TransitionType>& automaton, const string& nameFile) {

    ofstream fo (nameFile); 
    if (!fo.is_open()) {
        cerr << "Open file: failed" << endl;
        return; 
    }

    // States Printing...
    for (const set<int>& subState : automaton.getAllStates()) {
        for (int value : subState)
            fo << value << " ";
        fo << endl; 
    }

    // Final State Index Printing...
    for (const int& state: automaton.getFinalStates()) 
        fo << state << " ";
    fo << endl; 
 
    // Transition Printing...
    
    const set<char>& alphabet = automaton.getAlphabet();
    const auto& transitions = automaton.getTransitions();

    for (const State& state : automaton.getAllStates()) {
        for (char symbol : alphabet) {
            auto range = transitions.equal_range(make_pair(state, symbol));
            if (range.first != transitions.end())
                for (auto it = range.first; it != range.second; ++it)
                    fo << getIndexForState(automaton, it->second);
            fo<<endl; 
        }
    }
    fo.close(); 
}

#ifdef DOT

/**
 * @brief generation of .dot representation for non-deterministic finite automaton
 * 
 * @param automaton Automaton object 
 * @param imageName path and nameFile 
 */
template <typename State, typename Symbol, typename TransitionType>
void generateNDFADotFile(const Automaton<State, Symbol, TransitionType>& automaton, const string& imageName) {
    ofstream dotfile("workDOT.dot");

    if (!dotfile.is_open()) {
        cerr << "Open file: failed" << endl;
        return; 
    }

    dotfile << "digraph Automaton {" << endl;
    dotfile << "    rankdir=LR;" << endl;

    dotfile << "    node [shape=circle];";

    for (int i = 0; i < automaton.countAllStates(); ++i)
        dotfile << " " << i;

    State finalState = automaton.getFinalStates();
    //cout<<"Stato finale: DOT" <<finalState<<endl; 

    dotfile << ";" << endl; // Sposta il punto e virgola qui
    dotfile << "    node [shape=doublecircle];";

    dotfile << " " << finalState << " [peripheries=2]" ;

    dotfile << ";" << endl;

    multimap<pair<State, char>, State> transitions = automaton.getTransitions();

    /*

    */

    for (const auto& entry : transitions)
        entry.first.second=='*' ? dotfile << "    " << entry.first.first << " -> " << entry.second << " [label=\"" << "\u03B5" << "\"];" << endl : dotfile << "    " << entry.first.first << " -> " << entry.second << " [label=\"" << entry.first.second << "\"];" << endl;
        
    dotfile << "}" << endl;
    dotfile.close();

    string command = "dot -Tpng workDOT.dot -o " + imageName;
    system(command.c_str());

    system("rm workDOT.dot");
}

string generateStateName(int& stateCounter) {
    return string(1, 'A' + stateCounter++);
}

/**
 * @brief generation of .dot representation for deterministic finite automaton
 *
 * @param automaton Automaton item 
 * @param imageName path and namefile
 */
template <typename State, typename Symbol, typename TransitionType>
void generateDFADotFile(const Automaton<State, Symbol, TransitionType>& automaton, const string& imageName) {
    
    ofstream dotFile("workDOT.dot");
    
    if (!dotFile.is_open()) {
        cerr << "Open file: failed" << endl;
        return; 
    }

    dotFile << "digraph DFA {" << endl;
    dotFile << "    rankdir=LR;" << endl;

    map<State, string> stateToLetter;
    map<char, string> symbolToLabel;

    int stateCounter = 0; // Inizia con la lettera A

    for (const auto& entry : automaton.getTransitions()) {
        const State& fromState = entry.first.first;
        const char symbol = entry.first.second;
        const State& toState = entry.second;

        if (stateToLetter.find(fromState) == stateToLetter.end()) {
            stateToLetter[fromState] = generateStateName(stateCounter);
        }

        if (stateToLetter.find(toState) == stateToLetter.end()) {
            stateToLetter[toState] = generateStateName(stateCounter);
        }

        string fromStateLetter = stateToLetter[fromState];
        string toStateLetter = stateToLetter[toState];

        string symbolStr(1, symbol);

        dotFile << "    " << fromStateLetter << " -> " << toStateLetter
                << " [label=\"" << symbolStr << "\"];" << endl;
    }

    dotFile << "}" << endl;
    
    string command = "dot -Tpng workDOT.dot -o " + imageName;
    system(command.c_str());
    system("rm workDOT.dot");

    dotFile.close();
}

#endif

int main(int argc, char** argv) {

    if (argc < 2) {
        cout << "insufficient arguments - please enter the file name or bye bye!!" << endl; 
        return -1;
    }

    Automaton<int, char, multimap<pair<int, char>, int>> NDFA = importNDFA (argv[1]);
    NDFA.printAll(); 

    Automaton<set<int>, char, map<pair<set<int>, char>, set<int>>> DFA = NDtoD(NDFA); 
    DFA.printAll(); 

    writeOutputDFA(DFA, "outputDFA.txt"); 
    
    #ifdef DOT
        generateNDFADotFile(NDFA, "NDFA.png"); 
        generateDFADotFile(DFA, "DFA.png");
    #endif
    
    return 0;
}