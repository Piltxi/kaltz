/**
 * @file ERtoND.cpp
 * @author Elia Pitzalis [284660@studenti.unimore.it]
 * @brief library of functions for the analysis of finite automata; includes Thompson's Construction Algorithm and image generation (via dot) of the automata
 * @version 0.2
 * @date 2023-11-04
 * 
 * @note g++ -std=c++11 to compile
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include <stack>
using namespace std;

set<char> operators = {'*','|','.'}; 
const char epsilon = '*'; // '*' simulates the Greek epsilon

// #define DEBUG
 #define DOT

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

    void addTransition (const State&, const Symbol&, const State&) {
        // default addTransition
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
    cout << "Alphabet: ";
    for (const char& symbol : alphabet)
        symbol=='*' ? cout << "" << "\u03B5" << " " : cout << "'" << symbol << "' ";
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
            entry.first.second==epsilon ? cout<< "fromState: " << entry.first.first <<" >"<<"\u03B5"<<endl : cout<< "fromState: " << entry.first.first <<" >"<<entry.first.second<<endl ;
            currentKey = entry.first;
        }
        cout << "\t-> " << entry.second << endl;
    }

    cout << "\nFinal State: " << getFinalStates()<<endl; 
    
    cout << endl; 
}

/**
 * @brief Customized method for add transitions in NDFA.
 *
 * This specialization of the printAll function is used to print detailed information about a non-deterministic finite automaton (NDFA).
 * This method is specialized for adding transitions where State is of type int, not set<int>. 
 * It ensures that the number of transitions for the input alphabet '*' is limited to 2, and for other input alphabets, only one transition is allowed.
 *
 * @note This specialized is tailored for NDFA objects with specific template parameters.
 * @param stateInReading The state from which the transition originates.
 * @param inputAlpha The input alphabet symbol for the transition.
 * @param toState The state to which the transition leads.
 * @throw runtime_error if the number of transitions for '*' exceeds 2 or if a duplicate transition for another alphabet is added.
 */
template <>
void Automaton<int, char, multimap<pair<int, char>, int>>::addTransition(const int& stateInReading, const char& inputAlpha, const int& toState) {

    auto starTransitionsRange = transitions.equal_range(make_pair(stateInReading, '*'));
    size_t starCount = distance(starTransitionsRange.first, starTransitionsRange.second);
    
    if (inputAlpha == '*') {
        if (starCount < 2) {
            transitions.emplace(make_pair(make_pair(stateInReading, inputAlpha), toState));
        } else {
            throw runtime_error ("\nERROR - Transitions number, addTransition\n");
        }
    } else {
        auto existingTransitionsRange = transitions.equal_range(make_pair(stateInReading, inputAlpha));
        size_t count = distance(existingTransitionsRange.first, existingTransitionsRange.second);
        
        if (count == 0) {
            transitions.emplace(make_pair(make_pair(stateInReading, inputAlpha), toState));
        } else {
            throw runtime_error ("\nERROR - Transitions number, addTransition\n");
        }
    }
}

/**
 * @brief defined Class to insert regular expressions
 */
class Regex {

    private: 
        set<char> alphabet; 
        string expression; 
    
    public: 

        Regex () {}; 

        const string& getExpression () const {
            return expression; 
        }

        const set<char>& getAlphabet () const{
            return alphabet; 
        }

        void setExpression (string expression) {
            this->expression = expression; 
        }

        void setAlphabet (set<char> alphabet) {
            this->alphabet = alphabet; 
        }

        void viewRegex () {

            cout<<"Alphabet:\n"; 
            for (const char& symbol : alphabet)
                cout<<symbol<<" "; 
            cout<<endl; 

            cout<<"Regular expression:\n"<<expression<<endl<<endl;
        }
}; 

/**
 * @brief defined Class to implement the abstract syntax tree.
 * Each object contains pointers to child objects (left, right). 
 * Implements the necessary and fundamental methods for using the tree.
 * 
 * Contains Review recursive method to print a regular expression according to linear representation
 */
class AST {

    private:
        char character; 
        AST* left; 
        AST* right; 
    
    public: 
        AST (char character, AST* left, AST* right): character(character), left(left), right(right) {};
        AST (char character): character(character), left(nullptr), right(nullptr) {};

        char getCharacter () const {
            return character; 
        }

        AST* getLeft () {
            return left; 
        }

        AST* getRight () {
            return right; 
        }

        string REview () {

            string ReRep = "";

            if (left != nullptr) {

                string leftRE = left->REview();

                if (right != nullptr) {

                    string rightRE = right->REview();
                    ReRep.push_back(character); 
                    
                    ReRep.push_back('(');
                    ReRep.append(leftRE); 
                    ReRep.push_back(')');

                    ReRep.push_back('(');
                    ReRep.append(rightRE); 
                    ReRep.push_back(')');

                }

                else {

                    ReRep.push_back(character); 
                    
                    ReRep.push_back('(');
                    ReRep.append(leftRE); 
                    ReRep.push_back(')');

                }
            }

            else 
                ReRep.push_back(character); 

            return ReRep; 
        }
    
}; 

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
 * @brief Function to acquire data on a regular expression (alphabet, expression) from a file. 
 * 
 * @param nameFile file name with regex
 * @return Regex regex object imported from file
 */
Regex importRegex (const char* nameFile) {

    Regex regex; 

    ifstream fi (nameFile); 
    if (!fi.is_open()) {
        cout<<"\nERROR - Opening file...\n";
        return regex; 
    }
    
    // Alphabet acquisition...
    string line; 
    getline(fi, line); 

    set<char>regexAlphabet; 

    stringstream alphabet(line); 
    char symb;
    while(alphabet>>symb)
            regexAlphabet.insert(symb);

        
    regex.setAlphabet(regexAlphabet); 

    // Expression acquisition...
    string expression; 
    getline(fi, expression); 
    
    regex.setExpression(expression); 

    return regex; 
}

/**
 * @brief Returns the first non-blank character starting from the specified position (inclusive).
 *
 * @param S The string to search for the non-blank character in.
 * @param j The position from which to start the search.
 * @return int The position of the first non-blank character found.
 */
int skipblank(string S, int j) {
    while (j < S.length() && S[j] == ' ') j++;
    return j;
}

/**
 * @brief Returns a "compacted" string where all blank characters are removed.
 *
 * This function removes all blank characters from the input string and returns the resulting string.
 *
 * @param S The string from which to remove blank characters.
 * @return string The "compacted" string without blank characters.
 */
string removeblank(string S) {
    int n = S.length();
    int j = skipblank(S, 0);
    string compact = "";
    while (j < n) {
        compact.push_back(S.at(j));
        j = skipblank(S, ++j);
    }
    return compact;
}

/**
 * @brief Calculates the length of the substring that correctly balances parentheses starting from the specified position.
 *
 * This function calculates the length of the substring within the input string that starts from the given position and correctly balances parentheses.
 *
 * @param S The string in which to find the balanced substring.
 * @param j The starting position from which to search for the balanced substring.
 * @return int The length of the balanced substring.
 */
int getSubTree(string S, int j) {
    int s = j;
    j++;
    int numpar = 1;
    
    while (numpar > 0) {
        if (S.at(j) == '(') numpar++;
        if (S.at(j) == ')') numpar--;
        j++;
    }
    return j - s;
}

/**
 * @brief Builds an Abstract Syntax Tree (AST) for a given regular expression.
 *
 * This function constructs an Abstract Syntax Tree (AST) from a given regular expression and a corresponding alphabet.
 *
 * @param regex The regular expression object that provides the alphabet.
 * @param inputRegex The input regular expression to construct the AST from.
 * @return AST* A pointer to the root of the constructed AST.
 * @throw invalid_argument if an unknown character or operator is encountered in the input.
 */
AST* buildSyntaxTree (const Regex& regex, string inputRegex) {

    int lenght = inputRegex.size(); 

    if (lenght == 1 || lenght == 3) {

        char character = inputRegex.at(max(0, lenght-2));
        
        if (regex.getAlphabet().count(character) == 0)
        throw invalid_argument ("unknow "+string(1,character)+"...");

        return new AST (character); 
    }

    char op = inputRegex.at(1); 
    
    if (operators.count(op) == 0) 
    throw invalid_argument("unknow "+string(1,op)+" operator...");

    int startIndex = 2; 
    int stopIndex = getSubTree(inputRegex, startIndex); 

    string leftRegex = inputRegex.substr(startIndex, stopIndex);
    startIndex+=stopIndex; 

    if (op == '*') {

        AST* leftTree = buildSyntaxTree (regex, leftRegex);
        return new AST (op, leftTree, nullptr); 
    }

    else {
        
        stopIndex = getSubTree (inputRegex, startIndex);
        string rightRegex = inputRegex.substr(startIndex, stopIndex);

        AST* leftTree = buildSyntaxTree (regex, leftRegex);
        AST* rightTree = buildSyntaxTree (regex, rightRegex);
        
        return new AST (op, leftTree, rightTree); 
    }
}

/**
 * @brief Traverses an Abstract Syntax Tree (AST) and returns a stack of AST nodes.
 *
 * @param abstractSyntaxTree The root of the Abstract Syntax Tree to traverse.
 * @return stack<AST*> stack with AST nodes by DFS visit
 * @throw invalid_argument if the input AST is null (empty tree)
 */
stack<AST*> visitingASTforAutoma (AST* abstractSyntaxTree) {

    if (abstractSyntaxTree == nullptr) 
        throw invalid_argument ("\nERROR - visitingASTforAutoma\n");
    

    stack<AST*> working; 
    stack<AST*> end; 
    
    working.push(abstractSyntaxTree);
    while (!working.empty()) {
            
        AST* node = working.top();
            
        working.pop();
        end.push(node);

        if (node->getLeft() != nullptr)
            working.push(node->getLeft());

        if (node->getRight() != nullptr)
            working.push(node->getRight());
    }

    return end; 
}

/**
 * @brief The function creates a finite automaton to recognize a symbol. 
 * 
 * The returned automaton is elementary and belongs to the pool of basic automata.
 * 
 * @param symbol symbol for which to create recognition automaton
 * @param stateCounter starting state number
 * @return Automaton<int, char, multimap<pair<int, char>, int>> recognition automaton with transition to recognize symbol
 */
Automaton<int, char, multimap<pair<int, char>, int>> AutomataPool_Symbol (const char symbol, int& stateCounter) {

    Automaton<int, char, multimap<pair<int, char>, int>> ndfa;

    set<char> alphabet; 
    alphabet.insert(epsilon); 
    alphabet.insert(symbol); 
    ndfa.setAlphabet(alphabet);

    int finalState = stateCounter+1;
    ndfa.setInitialState(stateCounter);
    ndfa.setFinalStates(finalState);

    ndfa.addState(stateCounter);
    ndfa.addState(finalState);

    ndfa.addTransition(stateCounter, symbol, finalState);

    stateCounter+=2; 

    return ndfa; 
}

/**
 * @brief function that implements the * [Kleene Star] operator of an input automata
 * 
 * The returned automaton is elementary and belongs to the pool of basic automata.
 * rule involves closing a regular expression
 * 
 * @param left input automaton on which to implement operator *
 * @param stateCounter starting state number
 * @return final automaton with operator *
 */
Automaton<int, char, multimap<pair<int, char>, int>> AutomataPool_KleeneStar (const Automaton<int, char, multimap<pair<int, char>, int>>& left, int& stateCounter) {

    Automaton<int, char, multimap<pair<int, char>, int>> kleeneStar;

    set<char> alphabet;
    alphabet.insert(left.getAlphabet().begin(), left.getAlphabet().end());
    kleeneStar.setAlphabet(alphabet); 


    int newStartState = stateCounter; 
    int newFinalState = stateCounter + 1; 
    kleeneStar.setInitialState(newStartState); 
    kleeneStar.setFinalStates(newFinalState); 

    stateCounter+=2; 

    multimap<pair<int, char>, int> mergeTransitions; 
    mergeTransitions.insert(left.getTransitions().begin(), left.getTransitions().end());

    kleeneStar.setTransitions(mergeTransitions);

    // Start->End Automa [epsilon]
    kleeneStar.addTransition(newStartState, epsilon, newFinalState);

    // Old.END -> New.END [epsilon]
    kleeneStar.addTransition((left.getFinalStates()), epsilon, newFinalState); 

    // New.START -> Old.START [epsilon]
    kleeneStar.addTransition(newStartState, epsilon, (left.getStartState())); 

    // Old.END -> Old.START [epsilon]
    kleeneStar.addTransition((left.getFinalStates()), epsilon, (left.getStartState())); 

    for (const auto& entry : kleeneStar.getTransitions()) {
        kleeneStar.addState(entry.first.first); 
        kleeneStar.addState(entry.second);
    }

    return kleeneStar; 
}

/**
 * @brief function that implements the | operator [Union] of two input automata
 * 
 * The returned automaton is elementary and belongs to the pool of basic automata.
 * rules for union automata
 * 
 * @param left first input automaton (left pointer of parent object)
 * @param right second input automaton (right pointer of parent object)
 * @param stateCounter starting state number
 * @return Automaton<int, char, multimap<pair<int, char>, int>> final resutl automaton with operator |
 */
Automaton<int, char, multimap<pair<int, char>, int>> AutomataPool_Union (const Automaton<int, char, multimap<pair<int, char>, int>>& left, const Automaton<int, char, multimap<pair<int, char>, int>>& right, int& stateCounter) {

    #ifdef DEBUG
        cout<<"START printing input automata - union implementation: "<<endl; 
        cout<<"Left Automa:"; 
        left.printAll(); 
        cout<<"Right Automa:"; 
        right.printAll(); 
        cout<<"END printing input automata - union implementation..."<<endl; 
    #endif

    Automaton<int, char, multimap<pair<int, char>, int>> union_; 
    
    set<char> alphabet;
    alphabet.insert(left.getAlphabet().begin(), left.getAlphabet().end());
    alphabet.insert(right.getAlphabet().begin(), right.getAlphabet().end());
    union_.setAlphabet(alphabet); 
    
    int newStartState = stateCounter;
    stateCounter+=1;
    int newFinalState = stateCounter;
    stateCounter+=1;

    union_.setInitialState(newStartState); 
    union_.setFinalStates(newFinalState); 

    multimap<pair<int, char>, int> mergeTransitions; 
    mergeTransitions.insert(left.getTransitions().begin(), left.getTransitions().end());
    mergeTransitions.insert(right.getTransitions().begin(), right.getTransitions().end());
    union_.setTransitions(mergeTransitions);

    union_.addTransition(newStartState, epsilon, left.getStartState()); 
    union_.addTransition(newStartState, epsilon, right.getStartState()); 
    union_.addTransition(left.getFinalStates(), epsilon, newFinalState); 
    union_.addTransition(right.getFinalStates(), epsilon, newFinalState); 

    for (const auto& entry : union_.getTransitions()) {
        union_.addState(entry.first.first); 
        union_.addState(entry.second);
    }

    return union_; 
}

/**
 * @brief function that implements the . operator [Concanation] of two input automata
 * 
 * The returned automaton is elementary and belongs to the pool of basic automata.
 * rules for the concatenation of automata
 * 
 * @param left first input automaton (left pointer of parent object)
 * @param right second input automaton (right pointer of parent object)
 * @return Automaton<int, char, multimap<pair<int, char>, int>> final result automaton with operator .
 */
Automaton<int, char, multimap<pair<int, char>, int>> AutomataPool_Concatenation (const Automaton<int, char, multimap<pair<int, char>, int>>& left, const Automaton<int, char, multimap<pair<int, char>, int>>& right) {

    #ifdef DEBUG
        cout<<"START printing input automata - concatenation implementation: "<<endl; 
        cout<<"Left Automa:"; 
        left.printAll(); 
        cout<<"Right Automa:"; 
        right.printAll(); 
        cout<<"END printing input automata - concatenation implementation..."<<endl; 
    #endif

    Automaton<int, char, multimap<pair<int, char>, int>> concatenation; 

    set<char> alphabet;
    alphabet.insert(left.getAlphabet().begin(), left.getAlphabet().end());
    alphabet.insert(right.getAlphabet().begin(), right.getAlphabet().end());
    concatenation.setAlphabet(alphabet); 

    concatenation.setInitialState(left.getStartState()); 
    concatenation.setFinalStates(right.getFinalStates());

    multimap<pair<int, char>, int> mergeTransitions; 
    mergeTransitions.insert(left.getTransitions().begin(), left.getTransitions().end());
    mergeTransitions.insert(right.getTransitions().begin(), right.getTransitions().end());
    concatenation.setTransitions(mergeTransitions);
   
    concatenation.addTransition(left.getFinalStates(), epsilon, right.getStartState()); 

    for (const auto& entry : concatenation.getTransitions()) {
        concatenation.addState(entry.first.first); 
        concatenation.addState(entry.second);
    }

    return concatenation; 
}

/**
 * @brief Converts a Regular Expression (RE) represented by an Abstract Syntax Tree (AST) into a Non-Deterministic Finite Automaton (NDFA).
 *
 * This function takes an AST representing a regular expression and converts it into a Non-Deterministic Finite Automaton (NDFA).
 *
 * @param abstractSyntaxTree The root of the Abstract Syntax Tree representing the regular expression.
 * @param regex The regular expression in the form of a Regex object.
 * @return Automaton<int, char, multimap<pair<int, char>, int>> The resulting Non-Deterministic Finite Automaton (NDFA).
 * @throw invalid_argument if the input AST is nullptr.
 * @throw runtime_error if the regular expression is invalid or cannot be converted into an NDFA.
 */
Automaton<int, char, multimap<pair<int, char>, int>> REtoND(AST* abstractSyntaxTree, const Regex& regex) {

    if (abstractSyntaxTree == nullptr)
        throw invalid_argument("\nERROR - abstractSyntaxTree, REtoND\n");

    set<char> alphabet = regex.getAlphabet();

    stack<AST*> visitingAST = visitingASTforAutoma(abstractSyntaxTree);

    stack<Automaton<int, char, multimap<pair<int, char>, int>>> automatons;
    int stateCounter = 0;

    while (!visitingAST.empty()) {

        #ifdef DEBUG
            cout<<"stateCounter: "<<stateCounter<<endl; 
        #endif
        
        AST* _operator = visitingAST.top();
        visitingAST.pop();

        if (alphabet.find(_operator->getCharacter()) != alphabet.end()) {
           
            Automaton<int, char, multimap<pair<int, char>, int>> newAutoma = AutomataPool_Symbol(_operator->getCharacter(), stateCounter);

             #ifdef DEBUG
                cout<<"Automaton from symbol printing..."; 
                newAutoma.printAll(); 
            #endif

            automatons.push(newAutoma);
        } 

        else {

            switch (_operator->getCharacter()) {
                
                case '|': {

                    if (automatons.size() < 2)
                        throw runtime_error("Invalid union operator.");

                    Automaton<int, char, multimap<pair<int, char>, int>> rightNDFA = automatons.top();
                    automatons.pop();
                    Automaton<int, char, multimap<pair<int, char>, int>> leftNDFA = automatons.top();
                    automatons.pop();

                    Automaton<int, char, multimap<pair<int, char>, int>> newAutoma = AutomataPool_Union(leftNDFA, rightNDFA, stateCounter); 

                    #ifdef DEBUG
                        cout<<"Automaton from union printing..."; 
                        newAutoma.printAll(); 
                    #endif

                    automatons.push(newAutoma);
                    break;
                }
                
                case '*': {

                    if (automatons.size() < 1)
                        throw runtime_error("Invalid Kleene star operator.");

                    Automaton<int, char, multimap<pair<int, char>, int>> leftNDFA = automatons.top();
                    automatons.pop();
                    
                    Automaton<int, char, multimap<pair<int, char>, int>> newAutoma = AutomataPool_KleeneStar(leftNDFA, stateCounter); 

                    #ifdef DEBUG
                        cout<<"Automaton from kleeneStar printing..."; 
                        newAutoma.printAll(); 
                    #endif

                    automatons.push(newAutoma);
                    break;
                }
                
                case '.': {
                    if (automatons.size() < 2)
                        throw runtime_error("Invalid concatenation operator.");

                    Automaton<int, char, multimap<pair<int, char>, int>> rightNDFA = automatons.top();
                    automatons.pop();
                    Automaton<int, char, multimap<pair<int, char>, int>> leftNDFA = automatons.top();
                    automatons.pop();
                    
                    Automaton<int, char, multimap<pair<int, char>, int>> newAutoma = AutomataPool_Concatenation(leftNDFA, rightNDFA); 

                    #ifdef DEBUG
                        cout<<"Automaton from concatenation printing..."; 
                        newAutoma.printAll(); 
                    #endif

                    automatons.push(newAutoma);
                    break;
                }

                default:
                    throw invalid_argument("\nERROR - operator, REtoND\n");
            }
        }
    }

    if (automatons.size() != 1)
        throw runtime_error("\nERROR - Invalid regular expression, REtoND\n");

    Automaton<int, char, multimap<pair<int, char>, int>> resultNDFA = automatons.top();

    return resultNDFA; 
}

#ifdef DOT
/**
 * @brief Writes the NDFA (Non-deterministic Finite Automaton) represented by the given automaton to a file.
 *
 * This function writes the state information, final states, and transitions of a NDFA to a text file.
 * The data is formatted according to the homework standard requirements.
 * 
 * @param automaton The NDFA to be written to the file.
 * @param imageName The name of the output image where the NDFA will be saved.
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

    dotfile << ";" << endl; // Sposta il punto e virgola qui
    dotfile << "    node [shape=doublecircle];";

    dotfile << " " << finalState << " [peripheries=2]" ;

    dotfile << ";" << endl;

    multimap<pair<State, char>, State> transitions = automaton.getTransitions();

    for (const auto& entry : transitions)
        entry.first.second=='*' ? dotfile << "    " << entry.first.first << " -> " << entry.second << " [label=\"" << "\u03B5" << "\"];" << endl : dotfile << "    " << entry.first.first << " -> " << entry.second << " [label=\"" << entry.first.second << "\"];" << endl;
        
    dotfile << "}" << endl;
    dotfile.close();

    string command = "dot -Tpng workDOT.dot -o " + imageName;
    system(command.c_str());

    system("rm workDOT.dot");
    cout<<"output ndfa [dot rappresentation] exported in file "<<imageName<<endl; 
}
#endif

/**
 * @brief Writes the NDFA (Non-deterministic Finite Automaton) represented by the given automaton to a file.
 *
 * This function writes the state information, final states, and transitions of a NDFA to a text file.
 * The data is formatted according to the homework standard requirements.
 * 
 * @param automaton The NDFA to be written to the file.
 * @param nameFile The name of the output file where the NDFA will be saved.
 */
template <typename State, typename Symbol, typename TransitionType>
void writeOutputNDFA (const Automaton<State, Symbol, TransitionType>& automaton, const string& nameFile) {

    ofstream fo (nameFile); 
    if (!fo.is_open()) {
        cerr << "Open file: failed" << endl;
        return; 
    }

    // Alphabet printing...
    set<char> alphabet = automaton.getAlphabet();
    alphabet.erase(epsilon);
    for (const char& symbol : alphabet)
        fo << symbol << " ";
    fo<<endl; 

    // Final state printing...
    fo << automaton.getFinalStates()<<endl; 

    // Transitions printing...
    multimap< pair<int, char>, int> transitions = automaton.getTransitions(); 
    alphabet = automaton.getAlphabet();

    for (int i = 0; i < automaton.countAllStates(); ++i){
        for (const char& symbol : alphabet) {

            //pair<i, symbol> key; 
            auto range = transitions.equal_range(make_pair(i, symbol)); 

            if (range.first == range.second)
               { fo<<endl; continue;}
            
            else 
                for (auto it = range.first; it != range.second; ++it)
                    fo << it->second << " "; 
            fo<<endl; 
        }
    }
    fo.close();
    cout<<"output ndfa exported in file "<<nameFile<<endl; 
}   

int main (int argc, char** argv) {

    if (argc < 2) {
        cout << "insufficient arguments - please enter the file name or bye bye!!" << endl; 
        return -1;
    }

    Regex regex = importRegex(argv[1]);
    AST *ast = buildSyntaxTree (regex, removeblank(regex.getExpression()));

    #ifdef DEBUG
        cout<<"Regex from Abstract Syntax Tree: \n\t"<<ast->REview()<<endl; 
    #endif

    Automaton<int, char, multimap<pair<int, char>, int>> ND = REtoND (ast, regex);
    ND.printAll();
    
    #ifdef DOT
        generateNDFADotFile(ND, "NDFA.png"); 
    #endif

    writeOutputNDFA(ND, "outNDFA.txt");

    return 0; 
}