#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<algorithm>
#include<cmath>
#include<omp.h>
#include <iomanip>
using namespace std;

vector<string> fixedcode(128);

struct node
{
    int node_number;
    int value;
    char c;
    bool isNYT;
    node* left;
    node* right;
};

bool find(node* root, char ch, string &temp)
{
    if(root==NULL)
        return false;
    
    if(root->left==NULL && root->right==NULL)
    {
        if(root->isNYT==false && root->c==ch)
        {
            root->value++;
            return true;
        }
        else 
            return false;
    }
    
    if(find(root->left,ch,temp))
    {
        temp+='0';
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        root->value=root->left->value+root->right->value;
        return true;
    }

    if(find(root->right,ch,temp))
    {
        temp+='1';
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        root->value=root->left->value+root->right->value;
        return true;
    }

    return false;
}

bool NYTCode(node* root, string &temp, char ch)
{
    if(root==NULL)
        return false;
    
    if(root->left==NULL && root->right==NULL && root->isNYT==true)
    {
        root->isNYT=false;

        node* l = new node;
        node* r = new node;

        l->c='\0';
        l->isNYT=true;
        l->left=NULL;
        l->node_number=1;
        l->right=NULL;
        l->value=0;

        r->c=ch;
        r->isNYT=false;
        r->left=NULL;
        r->node_number=1;
        r->right=NULL;
        r->value=1;

        root->left=l;
        root->right=r;
        root->value=1;
        return true;
    }
    
    if(NYTCode(root->left,temp,ch))
    {
        temp+='0';
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        root->value=root->left->value+root->right->value;

        return true;
    }
    if(NYTCode(root->right,temp,ch))
    {
        temp+='1';
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        root->value=root->left->value+root->right->value;

        return true;
    }

    return false;
}

string code(int e, int r, char ch)
{
    int k,bits;

    k=ch-'\0';

    if(k>=0 && k<=2*r)
    {
        k--;
        bits=e+1;
    }
    else
    {
        k=k-r-1;
        bits=e;
    }
    string fixed_code;
    while(bits>0)
    {
        if(k%2==0)
            fixed_code+='0';
        else
            fixed_code+='1';
        k=k/2;
        bits--;
    }
            
    reverse(fixed_code.begin(),fixed_code.end());
    return fixed_code;
}

vector<string> Encoder(string &s, int &e, int &r, int &p)
{
    // vector chứa chuỗi mã hóa của từng thread 
    vector<string> encoded_string(p);
    // số lượng thread
    omp_set_num_threads(p);

    #pragma omp parallel // parallel region 
    {
        int me=omp_get_thread_num(); // ID của thread hiện tại 
        int nt=omp_get_num_threads(); // tổng số thread đang chạy 
        
        // khởi tạo cây huffman riêng cho mỗi thread 
        node* root = new node;
        root->node_number=256;
        root->value=0;
        root->c='\0';
        root->isNYT=true;
        root->left=NULL;
        root->right=NULL;

        // input 
        int n=s.length();
        int block=n/p; // chia chuỗi đầu vào cho p thread
        int i;

        // chuỗi mã hóa riêng của thread hiện tại 
        string encoded_me;

        // mảng đánh dấu ký tự đã xuất hiện
        vector<bool> m(128,0);

        // thread không phải thread cuối cùng
        if(me!=p-1)
        {
            // mỗi thread xử lý đoạn s[from...to)
            for(i=me*block;i<(me+1)*block;i++)
            {
                char mine;
                mine=s[i];

                string temp;
                // nếu ký tự đã xuất hiện trong block của thread 
                if(m[mine-'\0']==true)
                {
                    // tìm đường đi từ root tới node ký tự 
                    int isPresent=find(root,mine,temp);
                    reverse(temp.begin(),temp.end());
                    encoded_me+=temp;
                }
                else
                {
                    // ký tự mới xuất hiện trong block của thread
                    temp.clear();
                    // lấy mã đường đi từ NYT + update cây 
                    int z=NYTCode(root,temp,mine);
                    reverse(temp.begin(),temp.end());
                    temp+=fixedcode[mine-'\0'];
                    encoded_me+=temp;
                }
                // đánh dấu ký tự đã xuất hiện
                m[mine-'\0']=true;
            }
        }
        else
        {
            // thread cuối xử lý phần còn lại - khi n không chia hết cho p
            for(i=me*block;i<n;i++)
            {
                char mine;
                mine=s[i];

                string temp;
                
                if(m[mine-'\0']==true)
                {
                    int isPresent=find(root,mine,temp);
                    reverse(temp.begin(),temp.end());
                    encoded_me+=temp;
                }
                else
                {
                    temp.clear();
                    int z=NYTCode(root,temp,mine);
                    reverse(temp.begin(),temp.end());
                    temp+=fixedcode[mine-'\0'];
                    
                    encoded_me+=temp;
                }
                m[mine-'\0']=true;
            }
        }
        // vùng ghi chung, ghép kết quả mã hóa của từng thread vào mảng chung 
        #pragma omp critical        //critical region to collect encoded string from each procs
        {
            encoded_string[me]=encoded_me;
        }

        m.clear();
    }

    return encoded_string;
}

char find_leaf(string &encoded, int &i, node* root, node* &tnode)
{
    if(root->left==NULL && root->right==NULL)
    {
        if(root->isNYT==false)
        {
            root->value++;
            return root->c;
        }
        else
        {
            root->isNYT=false;

            node* l = new node;
            node* r = new node;

            l->c='\0';
            l->isNYT=true;
            l->left=NULL;
            l->node_number=1;
            l->right=NULL;
            l->value=0;

            r->c='k';
            r->isNYT=false;
            r->left=NULL;
            r->node_number=1;
            r->right=NULL;
            r->value=1;
            tnode=r;

            root->left=l;
            root->right=r;
            root->value=1;

            return '\0';
        }
    }
    if(encoded[i]=='0')
    {  
        i++;
        char k=find_leaf(encoded,i,root->left,tnode);
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        root->value=root->left->value+root->right->value;

        return k;
    }
    else
    {
        i++;
        char k=find_leaf(encoded,i,root->right,tnode);
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        root->value=root->left->value+root->right->value;

        return k;
    }
    if(root->right->value<root->left->value)
    {
        node* t=new node;
        t=root->left;
        root->left=root->right;
        root->right=t;
    }
    root->value=root->left->value+root->right->value;

    return '\0';
}

int calculate(string &encoded, int &i, int &e)
{
    int j=0;
    int num=0;
    while(j<e)
    {
        if(encoded[i]!='0')
        {
            num+=pow(2,e-j-1);
        }
        i++;
        j++;
    }
    return num;
}

string Decoder(vector<string> &encoded, int &e, int &r, int &p)
{
    // vector chứa chuỗi giải mã của từng thread
    vector<string> decoded(p);

    #pragma omp parallel // mỗi thread giải mã đúng phần encoded[thread_id]
    {

        int me=omp_get_thread_num(); // ID của thread hiện tại

        // khởi tạo cây huffman riêng cho mỗi thread
        node* root = new node;
        root->node_number=256;
        root->value=0;
        root->c='\0';
        root->isNYT=true;
        root->left=NULL;
        root->right=NULL;

        node* tnode=root; // con trỏ duyệt cây 
        int i=0;
        int n=encoded[me].length(); // độ dài của chuỗi mã hóa mà thread hiện cần decode 

        char dec;

        while(i<n)
        {
            dec=find_leaf(encoded[me],i,root,tnode);
            if(dec=='\0')
            {
                int num=calculate(encoded[me],i,e);
                char nw;
                if(num<r)
                {
                    num=num*2;
                    if(encoded[me][i]=='1')
                        num++;
                    
                    num++;
                    nw=num+'\0';
                    i++;
                    decoded[me]+=nw;
                }
                else
                {
                    num=num+r+1;
                    nw=num+'\0';
                    decoded[me]+=nw;
                }
                tnode->c=nw;
            }
            else
                decoded[me]+=dec;
        }
    }

    // ghép các kết quả giải mã của từng thread 
    string ans;
    int i;

    for(i=0;i<p;i++) 
    {
        ans+=decoded[i]; 
    }
        

    return ans;
}

int main(int argc, char* argv[]) 
{
    int e=6,r=64,p=16; // p: số lượng processors
    cout<<"Using "<<p<<" threads"<<endl;

    fixedcode.assign(128, "");
    for(int i=0;i<128;i++)
    {
        fixedcode[i]=code(e,r,(char)i);
    }

    string s,t; 

    // Default file nếu không có argument
    string filename = "../Tests/test.txt";
    if(argc > 1) {
        filename = "../Tests/" + string(argv[1]) + ".txt";
    }
    
    cout << "Processing file: " << filename << endl;
    ifstream MyReadFile(filename);       //file-name
    while (getline (MyReadFile,t)) 
    {
        s+=t;
    }
    
    cout<<"Input length: "<<s.length()<<endl;
    // cout<<"Original string: \""<<s<<"\""<<endl;
    
    double start,end,total;

    start=omp_get_wtime();
    vector<string> encoded=Encoder(s,e,r,p);
    end=omp_get_wtime();

    total=end-start;

    int len=0;
    string full_encoded = "";
    for(int i=0;i<encoded.size();i++) {
        len+=encoded[i].length();
        full_encoded += encoded[i];
    }

    cout<<"Encoded Length: "<<len<<endl;
    // cout<<"Encoded string: "<<full_encoded<<endl;
    cout<<"Encoder Time: "<<total<<" seconds"<<endl;
    
    start=omp_get_wtime();
    string decoded=Decoder(encoded,e,r,p);
    end=omp_get_wtime();

    total=end-start;

    cout<<"Decoded Length: "<<decoded.length()<<endl;
    // cout<<"Decoded string: \""<<decoded<<"\""<<endl;
    cout<<"Decoder Time: "<<total<<" seconds"<<endl;

    
    if(s==decoded) {
        cout<<"Message Decoded Successfully"<<endl;
        float compression_ratio = (float)(s.length() * 8) / len;
        cout<<"Compression Ratio: "<<compression_ratio<<endl;
    }
    else {
        cout<<"Decoding Failed!"<<endl;
    }

    fixedcode.clear();
    return 0;
}
