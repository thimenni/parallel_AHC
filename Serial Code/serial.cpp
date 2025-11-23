#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<algorithm>
#include<cmath>
#include<omp.h>
#include <iomanip>
using namespace std;

// ASCII có 128 ký tự, từ 0 đến 127 
vector<string> fixedcode(128); // lưu trữ fixed code cho mỗi ký tự 


// TreeNode
struct node
{
    int node_number;
    int value;
    char c;
    bool isNYT;
    node* left;
    node* right;
};

// Hàm tìm ký tự, xây dựng huffman path và update cây
bool find(node* root, char ch, string &temp)
{
    // nếu node rỗng
    if(root==NULL)
        return false;
    // nếu node là lá, kiểm tra ký tự
    if(root->left==NULL && root->right==NULL)
    {
        // lá này không phải NYT và là ký tự cần tìm
        if(root->isNYT==false && root->c==ch)
        {
            root->value++; // w++
            return true; // mark find = true
        }
        else 
            return false; // là lá nhưng không phải ký tự cần tìm
    }
    // tìm ở nhánh trái 
    if(find(root->left,ch,temp))
    {
        // nếu thấy ký tự ở nhánh trái, thêm '0' vào path 
        temp+='0';
        // kiểm tra xem có cần swap 2 node con không? 
        // weight(right) < weight(left) => vi phạm sibling property
        if(root->right->value<root->left->value)
        {
            // swap 2 node con 
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        // update trọng số node hiện tại = tổng trọng số 2 node con 
        root->value=root->left->value+root->right->value;

        return true;
    }
    // tương tự, tìm ở nhánh phải
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

    return false; // false nếu không tìm thấy
}

// Hàm xây huffman path đến NYT node và update cây 
bool NYTCode(node* root, string &temp, char ch)
{
    // nếu node rỗng
    if(root==NULL)
        return false;
    // nếu node là lá và là NYT 
    if(root->left==NULL && root->right==NULL && root->isNYT==true)
    {
        // update NYT old -> internal node
        root->isNYT=false;

        node* l = new node;
        node* r = new node;

        // NYT new
        l->c='\0'; // k chứa ký tự - null 
        l->isNYT=true;
        l->left=NULL;
        l->node_number=1;
        l->right=NULL;
        l->value=0;
        
        // new symbol node
        r->c=ch;
        r->isNYT=false;
        r->left=NULL;
        r->node_number=1;
        r->right=NULL;
        r->value=1;

        // assign 2 node con vào NYT old -> internal node
        root->left=l;
        root->right=r;
        root->value=1; // w++ for internal node 
        return true; // hoàn thành tạo node mới 
    }
    // duyệt nhánh trái tìm NYT node 
    if(NYTCode(root->left,temp,ch))
    {
        // thêm '0' vào path nếu tìm thấy NYT node
        temp+='0';
        // nếu weight trái > weight phải -> swap
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        // update trọng số node hiện tại = tổng trọng số 2 node con 
        root->value=root->left->value+root->right->value;

        return true;
    }
    // tương tự với nhánh bên phải 
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

    return false; // không tìm thấy NYT node
}


// Hàm tạo fixed code cho ký tự ch với tham số e và r
string code(int e, int r, char ch)
{
    int k,bits;
    // chuyển ký tự ch (ASCII) thành số nguyên k 
    k=ch-'\0';

    if(k>=0 && k<=2*r)
    {
        k--;
        bits=e+1; // binary(k-1) với (e+1) bit
    }
    else
    {
        k=k-r-1;
        bits=e; // binary(k-r-1) với e bit
    }
    string fixed_code;
    // chuyển số nguyên k sang binary string dài "bits" bit
    while(bits>0)
    {
        if(k%2==0)
            fixed_code+='0';
        else
            fixed_code+='1';
        k=k/2; // dịch phải 1 bit 
        bits--;
    }
  
    reverse(fixed_code.begin(),fixed_code.end());

    return fixed_code;
}


// Hàm Encoder, mã hóa chuỗi s với tham số e và r
string Encoder(string &s, int &e, int &r)
{
    string encoded_string; // res 
    // khởi tạo cây huffman ban đầu - 1 node NYT duy nhất
    node* root = new node;
    root->node_number=256;
    root->value=0;
    root->c='\0';
    root->isNYT=true;
    root->left=NULL;
    root->right=NULL;

    vector<bool> m(128,false); // mảng đánh dấu ký tự đó đã xuất hiện hay chưa?

    int n=s.length();
    int i;

    for(i=0;i<n;i++)
    {
        string temp; // mã 0/1 tạm thời cho ký tự s[i]
        
        if(m[s[i]-'\0']==true) // ký tự đã xuất hiện trước đó
        {
            int isPresent=find(root,s[i],temp); // gọi hàm find() để tìm ký tự đó, xây huffman path từ root đến ký tự đó + update cây 
            reverse(temp.begin(),temp.end()); // đảo ngược path (vì find build từ dưới lên)
            encoded_string+=temp; // nối mã huffman vào res
        }
        else // ký tự mới
        {
            temp.clear();
            int z=NYTCode(root,temp,s[i]); // gọi hàm NYTCode() để xây huffman path đến NYT node + tạo node mới cho ký tự s[i] + update cây
            reverse(temp.begin(),temp.end()); // đảm bảo path đúng hướng từ root đến NYT
            temp+=fixedcode[s[i]-'\0']; // thêm fixed code của ký tự vào sau NYT-code 
            encoded_string+=temp; // NYT - code + fixed code
        }
        m[s[i]-'\0']=true; // đánh dấu ký tự đã xuất hiện
    }

    m.clear();

    return encoded_string; // trả về encoded string 
}


// Hàm để decode: đi theo bit 0/1 để tìm lá trong cây huffman
char find_leaf(string &encoded, int &i, node* root, node* &tnode)
{
    // nếu node hiện tại là node lá
    if(root->left==NULL && root->right==NULL)
    {
        // là lá chứa ký tự
        if(root->isNYT==false)
        {
            root->value++; // w++
            return root->c; // trả về ký tự tìm được
        }
        else // là NYT node, ký tự mới xuất hiện 
        {
            root->isNYT=false; // cập nhật NYT old -> internal node

            node* l = new node;
            node* r = new node;

            l->c='\0';
            l->isNYT=true;
            l->left=NULL;
            l->node_number=1;
            l->right=NULL;
            l->value=0;

            r->c='k'; // ký tự tạm thời, sẽ được cập nhật sau
            r->isNYT=false;
            r->left=NULL;
            r->node_number=1;
            r->right=NULL;
            r->value=1;
            tnode=r; // lưu tạm node ký tự mới để cập nhật ký tự sau này

            root->left=l;
            root->right=r;
            root->value=1;

            return '\0'; // báo hiệu ký tự mới xuất hiện
        }
    }
    // nếu chưa tới node lá -> đọc bit để đi L/R
    if(encoded[i]=='0')
    {  
        i++; // đọc 1 bit 
        // đi xuống nhánh trái
        char k=find_leaf(encoded,i,root->left,tnode);
        // đảm bảo sibling property 
        if(root->right->value<root->left->value)
        {
            node* t=new node;
            t=root->left;
            root->left=root->right;
            root->right=t;
        }
        // update weight cho node hiện tại 
        root->value=root->left->value+root->right->value;

        return k;
    }
    else // encoded[i] == '1'
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


// Hàm đọc e bit từ chuỗi encoded và chuyển thành số nguyên
int calculate(string &encoded, int &i, int &e)
{
    int j=0;
    int num=0;
    // đọc đúng e bit, từ vị trí i 
    while(j<e)
    {
        // nếu bit hiện tại là '1'
        if(encoded[i]!='0')
        {
            // thêm giá trị số theo trọng số nhị phân
            // vị trí bit: e-j-1 
            num+=pow(2,e-j-1);
        }
        i++; // sang bit tiếp theo 
        j++; // tăng số bit đã đọc 
    }
    return num; 
}


// Hàm decoder, giải mã chuỗi bit encoded thành chuỗi ký tự gốc
string Decoder(string &encoded, int &e, int &r)
{
    string decoded; // res giải mã 
    node* root = new node;
    root->node_number=256;
    root->value=0;
    root->c='\0';
    root->isNYT=true;
    root->left=NULL;
    root->right=NULL;

    int i=0;
    int n=encoded.length(); // tổng số bit 

    char dec; // ký tự vừa giải mã
    node* tnode = root; // node sẽ chứa ký tự mới khi gặp NYT 

    // duyệt toàn bộ chuỗi bit
    while(i<n)
    {
        // đi theo bit 0/1 để tìm node lá 
        // ký tự cũ -> return ch
        // mới -> trả '\0' và trả về tnode 
        dec=find_leaf(encoded,i,root,tnode);
        if(dec=='\0')
        {
            int num=calculate(encoded,i,e);
            
            char nw;
            if(num<r)
            {
                num=num*2;
                if(encoded[i]=='1')
                    num++;
                
                num++;
                nw=num+'\0';
                i++;
                decoded+=nw;
            }
            else
            {
                num=num+r+1;
                nw=num+'\0';
                decoded+=nw;
            }
            tnode->c=nw;
        }
        else
            decoded+=dec;
    }
    
    return decoded;
}


int main(int argc, char* argv[]) 
{
    int e=6,r=64;       // for ASCII symbol-set 

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
    // cout<<"=========================================="<<endl;

    // float start,end,total;
    double start, end, total;

    start=omp_get_wtime();
    string encoded=Encoder(s,e,r);
    end=omp_get_wtime();

    total=end-start;
    cout<<"Encoded Length: "<<encoded.length()<<endl;
    // cout<<"Encoded string: "<<encoded<<endl;
    cout<<"Encoder Time: "<<total<<" seconds"<<endl;
    // cout<<"=========================================="<<endl;
    
    start=omp_get_wtime();
    string decoded=Decoder(encoded,e,r);
    end=omp_get_wtime();

    total=end-start;

    cout<<"Decoded Length: "<<decoded.length()<<endl;
    // cout<<"Decoded string: \""<<decoded<<"\""<<endl;
    cout<<"Decoder Time: "<<total<<" seconds"<<endl;
    // cout<<"=========================================="<<endl;

    
    if(s==decoded) {
        cout<<"Message Decoded Successfully"<<endl;
        float compression_ratio = (float)(s.length() * 8) / encoded.length();
        cout<<"Compression Ratio: "<<compression_ratio<<endl;
    }
    else {
        cout<<"Decoding Failed!"<<endl;
    }

    fixedcode.clear();
    return 0;
}
