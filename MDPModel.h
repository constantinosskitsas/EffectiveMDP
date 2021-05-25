#include <iostream>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

// for convenience
using json = nlohmann::json;

using namespace std;

/*int max(int a, int b){
    if (a>=b) return a;
    else return b;
}*/

string printableParameters(vector<pair<string,array<double,2>>> params){
    string s = "[";
    for (auto& element:params){
        s = s + "(";
        s = s + element.first + "," + to_string(element.second[0]) + "," + to_string(element.second[1]) + ") ";
    }
    s = s + "]";
    return s;
}

//class State;

class QState{
public:
    pair<string,int> action;
    int num_taken;
    double qvalue;
    vector<int> transitions;
    vector<double> rewards;
    int num_states;

    QState(pair<string, int> actionn = make_pair("ab",0), int numstates = -1, double qvaluee = 0.0){
        action = actionn;
        num_taken = 0;
        qvalue = qvaluee;
        num_states = numstates;
        if (num_states > 0){
            for (int i=0; i< num_states;i++){
            transitions.push_back(0);
            rewards.push_back(0.0);
            }
        }

    }

    void update(int state_num, double reward){
        num_taken++;
        transitions[state_num] += 1;
        rewards[state_num] += reward;
    }


    pair<string, int> get_action(){
        return action;
    }

    double get_qvalue(){
        return qvalue;
    }

    bool has_transition(int state_num){
        return (transitions[state_num] > 0);
    }


    double get_transition(int state_num){
        if (num_taken == 0){
            return (1.0 /num_states);
        }
        else{
            return ((double)transitions[state_num] /(double)num_taken);
        }
    }

    int get_num_transitions(int state_num){
        return transitions[state_num];
    }


    double get_reward(int state_num){
        if (transitions[state_num] == 0)
            return 0.0;
        else
            return (rewards[state_num] / (double)transitions[state_num]);
    }

    void set_qvalue(double qvaluee){
        qvalue = qvaluee;
    }

    int get_num_taken(){
        return num_taken;
    }


    vector<int> get_transitions(){
        return transitions;
    }

    vector<double> get_rewards(){
        return rewards;
    }

    friend ostream &operator<<( ostream &output, const QState& q);

};

ostream &operator<<( ostream &output, const QState& q){ 
         output << "Action: "<< q.action.first<<"\tQ-value: "<< q.qvalue << "\tTaken: "<< q.num_taken << endl;
         return output;            
}

class State{
public: 
    vector<QState> qstates;
    int state_num;
    int num_states;
    double value = 0.0;
    QState best_qstate;
    int num_visited = 0;
    vector<pair<string,array<double,2>>> parameters;

    State(vector<pair<string,array<double,2>>> parameterss = {}, int statenum = 0, double initialvalue = 0, int numstates = 0){
        state_num   = statenum;
        num_states  = numstates;
        value = initialvalue;
        parameters = parameterss;
    }

    void visit(){
        num_visited++;
    }

    int get_state_num(){
        return state_num;
    }

    void set_num_states(int numstates){
        num_states = numstates;
    }

    double get_value(){
        return value;
    }

    QState get_best_qstate(){
        return best_qstate;
    }

    pair<string,int> get_optimal_action(){
        return best_qstate.get_action();
    }

    int best_action_num_taken(){
        return best_qstate.get_num_taken();
    }

    void update_value(){

        best_qstate = qstates[0];
        value = qstates[0].get_qvalue();

        for (auto & element : qstates) {
            if (element.get_qvalue() > value){
                best_qstate = element;
                value = element.get_qvalue();
            }
        }
    }

    vector<pair<string,array<double,2>>> get_parameters(){
        return parameters;
    }

    void add_new_parameter(string name, array<double,2> values){
        parameters.push_back(make_pair(name, values));
    }

    pair<bool,array<double,2>> get_parameter(string param){
        for (auto& x:parameters){
            if (x.first == param)
                return make_pair(true, x.second);
        }
        std::array<double, 2> n;
        return make_pair(false, n);
    }



    void add_qstate(QState q){
        qstates.push_back(q);
        if (best_qstate.num_states == -1){
            best_qstate = q;
        }
    }

    vector<QState> get_qstates(){
        return qstates;
    }


    QState get_qstate(pair<string, int> action){

        for (auto & element : qstates){
            if (element.get_action() == action)
                return element;
        }
    }

   /*map<int,int> get_max_transitions(){
        map<int,int> trans;
        
        for (int i=0; i < num_states; i++){
            for (auto & element : qstates){
                if (element.has_transition(i)){
                    if (trans.find(i) != trans.end())
                        trans.insert(pair<int,int>(i, max(trans.at(i), element.get_transition(i))));
                    else
                        trans.insert(pair<int,int>(i, element.get_transition(i)));
                }
            }
        }
        return trans;
    }*/

    vector<pair<string, int>> get_legal_actions(){

        vector<pair<string, int>> actions;
        for (auto & element : qstates){
            actions.push_back(element.get_action());
        }
        return actions;
    }

    friend ostream &operator<<(ostream &output, State& s);

    void print_detailed(){
        cout << state_num << ": " << printableParameters(parameters) << ", visited: "<< num_visited << endl;
        for (auto& qs:this->get_qstates())
            cout << qs << endl;
    }


};

ostream &operator<<(ostream &output, State& s){
        output << s.state_num << ": " << printableParameters(s.parameters) <<endl;
        return output;
}

/*void QState::update(State new_state, double reward){
        num_taken++;
        int state_num = new_state.get_state_num();
        transitions[state_num] += 1;
        rewards[state_num] += reward;
    }*/

class MDPModel{
    public:
        double discount;
        vector<State> states = {State()};
        vector<string> index_params;
        State current_state;
        json parameters;
        double update_error = 0.01;
        string update_algorithm;
        int max_VMs;
        int min_VMs;

    MDPModel(json conf = json({})){
        string required_fields[4] = {"parameters", "actions", "discount", "initial_qvalues"};
        if (conf.contains("discount"))
        discount = conf["discount"];

        if (conf.contains("parameters"))
        parameters = _get_params(conf["parameters"]);
    
        for (auto& element : parameters.items()) {
            index_params.push_back(element.key());
            _update_states(element.key(), element.value());
        }

        int num_states = states.size();

        for (auto & element : states){
            element.set_num_states(num_states);
        }
        if (conf.contains("actions")){
        _set_maxima_minima(parameters, conf["actions"]);

        if (conf.contains("initial_qvalues"))
            _add_qstates(conf["actions"], conf["initial_qvalues"]);
        }
        update_algorithm  = "single_update";
            
    };

    void _assert_modeled_params(json conf){
        if (conf["actions"].contains("add_VMs") || conf["actions"].contains("remove_VMs")){
            if (!conf["parameters"].contains("numbers_of_VMs"))
                throw std::invalid_argument( "NUMBER OF VMS MISSING" );
        }
    };

    json _get_params(json par){
        json new_pars;
        for (auto& element : par.items()){
            new_pars[element.key()] = {};
            if (element.value().contains("values")){
                vector<pair<double,double>> values;
                for (auto& x:element.value()["values"]){
                    values.push_back(make_pair(x, x));
                }
                new_pars[element.key()]["values"] = values;
            }

            else if (element.value().contains("limits")){
                vector<pair<double,double>> values;
                vector<double> aux;
                for (auto& x:element.value()["limits"]) aux.push_back(x);
                for (int i = 1; i< aux.size(); i++ )
                    values.push_back(make_pair(aux[i-1] , aux[i]));
                new_pars[element.key()]["values"] = values;
            }
        }
        return new_pars;
    }

    void set_state(json measurements){
        current_state = _get_state(measurements);
    }

    void _update_states(string name, json new_parameter){
        int state_num = 0;
        vector<State> new_states;
        //State new_state;
        for (auto& x:new_parameter["values"]){
            for (auto& s:states){
                State new_state(s.get_parameters(), state_num);
                new_state.add_new_parameter(name, x);
                new_states.push_back(new_state);
                state_num++;
            }
        }
        states = new_states;
    }

    State _get_state(json measurements){
        for (auto& s:states){
            bool matches = true;
            for (auto& par:s.get_parameters()){
                double min_v = par.second[0];
                double max_v = par.second[1];
                if (measurements[par.first] < min_v || measurements[par.first] > max_v){
                    matches = false;
                    break;
                }
            }
            if (matches) return s;
        }
    }

    void _set_maxima_minima(json parameters, json acts){
        if (acts.contains("add_VMs") || acts.contains("remove_VMs")){
            vector<int> vms;
            for (auto& x:parameters["number_of_VMs"]["values"]){
                vms.push_back(x[0]);
            }
            max_VMs = *max_element(vms.begin(), vms.end());
            min_VMs = *min_element(vms.begin(), vms.end());

        }
    }

    void _add_qstates(json acts, double initq){
        int num_states = states.size();
        for (auto& action:acts.items()){
            for (auto& val:action.value()){
                pair<string,int> act = make_pair(action.key(), val);
                for (auto& s:states){
                        if (_is_permissible(s, act)){
                            QState q(act, num_states, initq);
                            s.add_qstate(q);
                        }

                }
            }
        }

        for (auto& s:states){
            s.update_value();
        }
    }

    bool _is_permissible(State s, pair<string,int> a){
        string action_type = a.first;
        int action_value = a.second;
        pair<bool, array<double,2>> param_values;
        if (action_type == "add_VMs"){
             param_values = s.get_parameter("number_of_VMs");
             if (param_values.first)
                return (max(param_values.second[0] , param_values.second[1]) + action_value <= max_VMs);
        }
        else if (action_type == "remove_VMs"){
             param_values = s.get_parameter("number_of_VMs");
             if (param_values.first)
                return (min(param_values.second[0] , param_values.second[1]) - action_value >= min_VMs);
        }
        else return true;
    }

    pair<string,int> suggest_action(){
        return current_state.get_optimal_action();
    }

    vector<pair<string, int>> get_legal_actions(){
        return current_state.get_legal_actions();
    }
    
    void update(pair<string,int> action, json measurements,double reward){
        current_state.visit();
        QState qstate = current_state.get_qstate(action);
        if (qstate.num_states == -1) return;
        State new_state = _get_state(measurements);
        qstate.update(new_state.get_state_num(), reward);
        if (update_algorithm == "single_update"){
            _q_update(qstate);
            current_state.update_value();
        }
        else{
            value_iteration();
        }
        current_state = new_state;
    }

    void _q_update(QState qstate){
        double new_qvalue = 0;
        double r;
        double t;
        for (int i=0; i < states.size(); i++){
            t = qstate.get_transition(i);
            r = qstate.get_reward(i);
            new_qvalue += t * (r + discount * states[i].get_value());
            //cout << "T: " << t << ", Reward: " << r << "Discount: " << discount << endl;
        }
        qstate.set_qvalue(new_qvalue);
    }

    void _v_update(State state){
        for (auto& qs:state.get_qstates())
            _q_update(qs);
        state.update_value();
    }

    void value_iteration(double error = -1){
       if (error < 0)
            error = update_error;
        bool repeat = true;
        double old_value;
        double new_value;
        while(repeat){
            repeat = false;
            for (auto& s:states){
                old_value = s.get_value();
                _v_update(s);
                new_value = s.get_value();
                if (abs(old_value - new_value) > error)
                    repeat = true;
            }
        }
    }

    vector<string> get_parameters(){
        return index_params;
    }
    
    double get_percent_not_taken(){
        double total = 0.0;
        double not_taken = 0.0;
        for (auto& s:states){
            for (auto& qs:s.get_qstates()){
                total = total + 1.0;
                if (qs.get_num_taken() == 0)
                    not_taken = not_taken + 1.0;
            }
        }
        return not_taken / total;
    }

    void print_model(bool detailed=false){
        for (auto& s:states){
            if (detailed){
                s.print_detailed();
                cout << endl;
            }
            else
                cout << s << endl;
        }
    }
        
};
