//
// Created by sinemora on 2020/4/7.
//
#include "../lexer/lexer.h"
#include <map>
using namespace std;
namespace Grammar{
    bool debug=false;
    struct Node;
    void eliSelfRecur(int mId);
    void eliPassRecur();
    void eliCommonPreffix();
    void getFirst(const Node &node);
    void getFirstSet();
    void getEndSet();
    void getSelectSet();
    void eliNone();
    /**
     * 表示串里的终结或非终结字符
     */
    struct Node{
        bool isEnd;
        int id;
        Node(int _id,bool _isEnd):id(_id),isEnd(_isEnd){}
        bool operator == (const Node &b) const{
            return isEnd==b.isEnd&&id==b.id;
        }
    };
    /**
     * 表示串，可以拼接、选取后缀等
     */
    struct Rule{
        vector<Node> rights;
        Rule getSubRule(int lp,int rp){
            Rule res;
            res.rights.clear();
            for(int i = lp;i <= rp;i++){
                res.rights.push_back(rights[i]);
            }
            return res;
        }
        Rule operator + (const Rule &b){
            Rule res;
            res.rights.clear();
            for(auto &t:rights)res.rights.push_back(t);
            for(auto &t:b.rights)res.rights.push_back(t);
            return res;
        }
        bool contain(const Rule &b){
            if(b.rights.size() > rights.size()) return false;
            for(int i = 0;i < b.rights.size();i++){
                if(!(b.rights[i] == rights[i])) return false;
            }
            return true;
        }
        Rule getSuffix(int p){
            Rule res;
            res.rights.clear();
            for(int i = p;i < rights.size();i++) {
                res.rights.push_back(rights[i]);
            }
            return res;
        }
        bool isNone() const{
            return rights.empty();
        }
    };
    /**
     * 中间节点信息
     */
    struct Mid{
        string sgn;//代表符号
        bool canNone=false,canEnd=false,vis=false;//first集是否有空字符，follow集是否有终结字符，vis用于算first时标记访问
        vector<Rule> rules;
        set<int> follow,first;
        map<int,int> select;
        Mid(){}
        Mid(string &_name):sgn(_name){}
    };
    /**
     * id串和节点标号互译
     */
    vector<string> ends;
    map<string,int> endToId;
    vector<Mid> mids;
    map<string,int> midToId;
    //map<string,int> cnts;
    /**
     * 语法树节点
     */
    struct TreeNode{
        vector<int> sons;
        string name,attr;//属性、值
        int lineNum;//行号
        bool hide=false;//用于删去推导为空的树节点
        TreeNode(){}
        TreeNode(int _n,string _name,string _attr):lineNum(_n),name(_name),attr(_attr){}
    };
    /**
     * 语法树
     */
    vector<TreeNode> resultTree(1);
    /**
     * 打印串信息
     * @param rule 串
     */
    void print(const Rule &rule){
        for(auto &t:rule.rights){
            if(t.isEnd) cout<<"<"<<ends[t.id]<<"> ";
            else cout<<mids[t.id].sgn<<" ";
        }
    }
    /**
     * 打印非终结节点信息
     * @param mid 非终结节点
     */
    void print(const Mid &mid){
        cout<<mid.sgn<<" -> ";
        for(auto &t:mid.rules){
            print(t);
            cout<<"|";
        }
        cout<<endl;
    }
    /**
     * 递归打印语法树
     * @param treeNode 语法树节点
     * @param deep 深度，用于缩进
     */
    void print(const TreeNode &treeNode,int deep){
        for(int i = 1;i < deep;i++){
            cout<<"  ";
        }
        cout<<treeNode.name;
        if(treeNode.attr!="_"&&treeNode.attr!=" "){
            cout<<" :"<<treeNode.attr;
        }
        cout<<"("<<treeNode.lineNum<<")"<<endl;
        for(int t:treeNode.sons){
            print(resultTree[t],deep+1);
        }
    }
    /**
     * 判断是不是某个非终结节点
     * @param node 字符
     * @param id 标号
     * @return 判断结果
     */
    bool judgeId(const Node &node,int id){
        return !node.isEnd && node.id == id;
    }
    /**
     * 节点转换为非终结节点
     * @param id 标号
     * @return 非终结节点
     */
    Node getNode(int id){
        return Node(id,false);
    }
    /**
     * 非终结符标号转换为串
     * @param id 非终结符标号
     * @return 串
     */
    Rule getRule(int id){
        Rule res;
        res.rights.clear();
        res.rights.push_back(getNode(id));
        return res;
    }
    /**
     * 获取所有终结节点的名称，并标号
     */
    void getEnds(){
        ends.clear();
        for(auto it:Lexer::endTran){
            ends.push_back(it.second);
            endToId[it.second] = (int)ends.size()-1;
        }
    }
    /**
     * 尝试添加非终结符，返回标号
     * @param midName 非终结符名字
     * @return 标号
     */
    int addMid(string midName){
        if(midToId.count(midName)) return midToId[midName];
        mids.emplace_back(midName);
        midToId[midName] = (int)mids.size()-1;
        return midToId[midName];
    }
    /**
     * 初始化
     * @param fileName 词法规则文件
     */
    void init(const string &fileName){
        getEnds();
        ifstream in;
        in.open(fileName);
        if(!in.is_open()){
            cerr<<"open "<<fileName<<" wrong"<<endl;
            return;
        }
        string nowString;
        Rule nowRule;
        int stat=0,nowMid=0;
        while(in>>nowString){
            if(nowString=="#"){
                stat = 0;
            }else if(nowString=="->"){
                stat = 1;
            }else if(stat==0){
                nowMid = addMid(nowString);
            }else if(nowString != "$"){
                if(Lexer::endTran.count(nowString)){
                    nowRule.rights.emplace_back(endToId[Lexer::endTran[nowString]],true);
                }else if(nowString == "none"){
                    mids[nowMid].canNone = true;
                }else{
                    nowRule.rights.emplace_back(addMid(nowString),false);
                }
            }else if(nowString == "$"){
                mids[nowMid].rules.push_back(nowRule);
                nowRule.rights.clear();
            }else{
                cerr<<"Parse Error"<<endl;
                return;
            }
        }
        eliPassRecur();
        eliCommonPreffix();
        eliNone();
        getFirstSet();
        getEndSet();
        getSelectSet();
        /*for(int i = 0;i < mids.size();i++){
            cout<<mids[i].sgn<<" -> ";
            for(auto t:mids[i].follow){
                cout<<ends[t]<<" ";
            }
            cout<<endl;
        }
        for(int i = 0;i < mids.size();i++){
            print(mids[i]);
        }*/
    }
    /**
     * 消除直接左递归
     * @param mId 非终结符标号
     */
    void eliSelfRecur(int mId){

        if(mids[mId].rules.empty()) return;
        vector<Rule> &now = mids[mId].rules;
        int spl = 0,n = now.size();
        for(int i = 0;i < n;i++){
            if(!now[i].rights.empty() && judgeId(now[i].rights[0],mId)){
                swap(now[i],now[spl]);
                spl++;
            }
        }
        if(spl==0)return;
        int nId = addMid(mids[mId].sgn+"'");
        vector<Rule> &nxt = mids[nId].rules;
        for(int i = 0;i < spl;i++){
            nxt.push_back(now[i].getSuffix(1)+getRule(nId));
        }
        mids[nId].canNone = true;
        nxt.emplace_back();
        for(int i = spl;i < n;i++){
            now[i] = now[i] + getRule(nId);
        }
        now.erase(now.begin(),now.begin()+spl);
    }
    /**
     * 消除间接左递归
     */
    void eliPassRecur(){
        vector<Rule> copyRule;
        for(int i = 0;i < mids.size();i++){
            for(int j = 0;j < i;j++){
                copyRule.clear();
                auto it = mids[i].rules.begin();
                while(it!=mids[i].rules.end()){
                    if(!((*it).rights.empty()) && judgeId((*it).rights[0],j)){
                        for(auto &tmp:mids[j].rules){
                            copyRule.push_back(tmp+(*it).getSuffix(1));
                        }
                    }else{
                        copyRule.push_back(*it);
                    }
                    ++it;
                }
                mids[i].rules = copyRule;
            }
            eliSelfRecur(i);
        }
    }
    /**
     * 获得两个串的最长公共前缀
     * @param a 串a
     * @param b 串b
     * @return 最长公共前缀
     */
    Rule getLCP(const Rule& a,const Rule& b){
        Rule res;
        res.rights.clear();
        if(a.isNone()||b.isNone()) return res;
        for(int i = 0;i < a.rights.size() && i < b.rights.size();i++){
            if(a.rights[i] == b.rights[i]) res.rights.push_back(a.rights[i]);
            else break;
        }
        return res;
    }
    /**
     * 消除公因子
     */
    void eliCommonPreffix(){
        int cnt=0;
        Rule LCP;
        for(int i = 0;i < mids.size();i++){
            bool flag = true;
            while(flag){
                flag = false;
                for(int j = 0;j < mids[i].rules.size();j++){
                    for(int k = j+1;k < mids[i].rules.size();k++){
                        LCP = getLCP(mids[i].rules[j],mids[i].rules[k]);
                        if(!LCP.isNone()){
                            flag=true;
                            int nId = addMid(mids[i].sgn+to_string(++cnt));
                            mids[nId].rules.push_back(mids[i].rules[j].getSuffix((int)LCP.rights.size()));
                            mids[nId].rules.push_back(mids[i].rules[k].getSuffix((int)LCP.rights.size()));
                            mids[i].rules[j] = LCP + getRule(nId);
                            mids[i].rules.erase(mids[i].rules.begin()+k);
                            for(int t = 0;t < mids[i].rules.size();t++){
                                if(t==j||t==k) continue;
                                if(mids[i].rules[t].contain(LCP)){
                                    mids[nId].rules.push_back(mids[i].rules[t].getSuffix((int)LCP.rights.size()));
                                    mids[i].rules.erase(mids[i].rules.begin()+t);
                                }
                            }
                        }
                        if(flag) break;
                    }
                    if(flag)break;
                }
            }
        }
    }
    /**
     * 扫描标记空生成
     */
    void eliNone(){
        for(auto &now:mids){
            auto it = now.rules.begin();
            while(it != now.rules.end()){
                if(it->rights.empty()){
                    now.canNone=true;
                    //it = now.rules.erase(it);
                    ++it;
                }else ++it;
            }
        }
    }
    /**
     * 获得串的first集
     * @param rule  串
     * @param canNone 是否有空字符
     * @param first 保存结果
     */
    void getFirst(const Rule &rule, bool &canNone,set<int> &first){
        bool flag = true;
        for(auto &nt:rule.rights){
            if(nt.isEnd) {first.insert(nt.id);flag=false;break;}
            else{
                getFirst(nt);
                for(auto it:mids[nt.id].first){

                    first.insert(it);
                }
                if(!mids[nt.id].canNone){
                    flag = false;
                    break;
                }
            }
        }
        if(flag) canNone = true;
    }
    /**
     * 获得非终结节点first集
     * @param node 节点
     */
    void getFirst(const Node &node){
        if(node.isEnd) return;
        if(mids[node.id].vis) return;
        mids[node.id].vis = true;
        for(auto &t:mids[node.id].rules){
            getFirst(t,mids[node.id].canNone,mids[node.id].first);
        }
    }
    /**
     * 获取所有节点first集
     */
    void getFirstSet(){
        int n = (int)mids.size();
        for(int i = 0;i < n;i++){
            getFirst(getNode(i));
        }
    }
    /**
     * 获得所有节点follow集
     */
    void getEndSet(){
        mids[0].canEnd = true;
        bool flag = true;
        while(flag){
            flag = false;
            for(int i = 0;i < mids.size();i++){
                for(int j = 0;j < mids.size();j++){
                    for(int tt = 0;tt < mids[j].rules.size();tt++) {
                        for (int k = 0; k < mids[j].rules[tt].rights.size(); k++) {
                            Node &now = mids[j].rules[tt].rights[k];
                            if (now == getNode(i)) {
                                bool canNone = false;
                                set<int> first;
                                getFirst(mids[j].rules[tt].getSuffix(k + 1), canNone, first);
                                /*cout << mids[i].sgn << " : ";
                                print(mids[i].rules[j].getSuffix(k + 1));
                                cout << endl;*/
                                if (canNone) {
                                    if (mids[j].canEnd && !mids[i].canEnd) {
                                        flag = true;
                                        mids[i].canEnd = true;
                                    }
                                    for (auto t:mids[j].follow) {
                                        if (!mids[i].follow.count(t)) {
                                            flag = true;
                                            mids[i].follow.insert(t);
                                        }
                                    }
                                }
                                for (auto t:first) {
                                    if (!mids[i].follow.count(t)) {
                                        flag = true;
                                        mids[i].follow.insert(t);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /**
     * 获得所有节点select集
     */
    void getSelectSet(){
        for(auto &mid:mids){
            for(int j = 0;j < mid.rules.size();j++){
                bool canNone=false;
                set<int> first;
                getFirst(mid.rules[j],canNone,first);
                for(auto t:first){
                    if(mid.select.count(t)){
                        cout<<"fucker "<<mid.sgn<<" "<<ends[t]<<endl;
                        return;
                    }
                    mid.select[t] = j;
                }
                if(canNone){
                    for(auto t:mid.follow){
                        if(mid.select.count(t)){
                            cout<<"fucker "<<mid.sgn<<" "<<ends[t]<<endl;
                            return;
                        }
                        mid.select[t] = j;
                    }
                }
            }
            /*
            cout<<mid.sgn<<" -> ";
            for(auto &t:mid.select){
                cout<<"("<<ends[t.first]<<","<<t.second<<")";
            }
            cout<<endl;
             */
        }
    }
    /**
     * 语法树遍历
     * @param mid 非终结节点
     * @param treeNode  语法树节点
     * @param pos 当前词法结果数组位置
     * @param lex 词法结果数组
     * @return 0表示成功，-1表示失败
     */
    int extend(Mid &mid,int treeNode,int &pos,vector<Lexer::result> &lex){
        resultTree[treeNode].hide=true;
        if(pos >= lex.size()){
            if(mid.canNone) return 0;
            cerr<<"Error :nothing to read!"<<endl;
            return -1;
        }
        if(!mid.select.count(endToId[lex[pos].type])){
            cerr<<"Error at Line "<<Lexer::lineRec[pos]<<": can't select "<<lex[pos].type<<" from "<<mid.sgn<<endl;
            return -1;
        }
        Rule &rule = mid.rules[mid.select[endToId[lex[pos].type]]];
        for(auto &node:rule.rights){
            if(node.isEnd && node.id==endToId[lex[pos].type]){
                resultTree.emplace_back(Lexer::lineRec[pos],lex[pos].type,lex[pos].value);
                resultTree[treeNode].sons.push_back((int)resultTree.size()-1);
                //cout<<resultTree.size()<<endl;
                pos++;
            }else if(!node.isEnd){
                resultTree.emplace_back(Lexer::lineRec[pos],mids[node.id].sgn,"_");
                resultTree[treeNode].sons.push_back((int)resultTree.size()-1);
                //cout<<treeNode.sons.size()<<endl;
                int ret = extend(mids[node.id],(int)resultTree.size()-1,pos,lex);
                if(ret==-1)return -1;
            }else{
                cerr<<"Error at Line "<<Lexer::lineRec[pos]<<": wrong sign "<<lex[pos].type<<" from "<<mid.sgn<<endl;
            }
        }
        for(auto t:resultTree[treeNode].sons){
            if(!resultTree[t].hide){
                resultTree[treeNode].hide = false;
                break;
            }
        }
        return 0;
    }
    /**
     * 消除不需要展示的节点
     * @param treeNode 语法树节点
     */
    void eliHide(TreeNode &treeNode){
        auto it = treeNode.sons.begin();
        while(it!=treeNode.sons.end()){
            if(resultTree[*it].hide){
                it = treeNode.sons.erase(it);
            }else{
                ++it;
            }
        }
    }
    /**
     * 分析生成语法树
     * @param lex 词法分析结果
     */
    void goGrammar(vector<Lexer::result> &lex){
        resultTree[0].lineNum = 0;
        resultTree[0].name = mids[0].sgn;
        resultTree[0].attr = "_";
        int pos = 0;
        if(extend(mids[0],0,pos,Lexer::LexRes)==-1)return;
        for(int i = 0;i < resultTree.size();i++) eliHide(resultTree[i]);
        print(resultTree[0],1);

    }
}