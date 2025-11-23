#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<algorithm>
#include<cmath>
#include <omp.h>
#include<mpi.h>
#include <queue>
#include <iomanip>
using namespace std;

struct node
{
    int node_number;
    int value;
    char c;
    bool isNYT;
    node* left;
    node* right;
};

// Hàm tìm ký tự đã xuất hiện trong cây huffman, xây dựng huffman path và update cây
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

// Hàm xử lý khi gặp ký tự mới 
bool NYTCode(node* root, string &temp, char ch)
{
    if(root==NULL)
        return false;
    
    // nếu node là lá và là NYT
    if(root->left==NULL && root->right==NULL && root->isNYT==true)
    {
        // tạo 2 node con mới cho node NYT hiện tại
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
    // tìm NYT node ở nhánh trái
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
    // tìm NYT node ở nhánh phải
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

// Hàm tăng value - weight cho node lá có ký tự là ch
void add_freq(node* root, char ch)
{
    if(root==NULL)
        return ;
    // node hiện tại là lá
    if(root->left==NULL && root->right==NULL)
    {
        // nếu ký tự đúng thì tăng trọng số
        if(root->c==ch)
            root->value++;
        
        return ;
    }

    add_freq(root->left,ch);
    add_freq(root->right,ch);
}

// Hàm xây dựng lại cây huffman để đảm bảo sibling property
void build(node* root)
{
    // root là lá
    if(root->left==NULL && root->right==NULL)
        return ;
    // gọi đệ quy để build subtree trái/phải
    build(root->left);
    build(root->right);
    // đảm bảo sibling property
    if(root->right->value<root->left->value)
    {
        node* t=new node;
        t=root->left;
        root->left=root->right;
        root->right=t;
    }
    // cập nhật giá trị cho node hiện tại
    root->value=root->left->value+root->right->value;

    return ;
}

// Hàm hoán đổi 2 node con để đảm bảo sibling property
void swap(node* root)
{
    if(root->left==NULL && root->right==NULL)
        return ;
    
    swap(root->left);
    swap(root->right);

    if(root->right->value<root->left->value)
    {
        node* t=new node;
        t=root->left;
        root->left=root->right;
        root->right=t;
    }

    return ;
}

// Hàm tạo mã cố định cho ký tự mới
string code(string temp, int e, int r, char ch)
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
    temp+=fixed_code;

    return temp;
}

// Hàm đánh node number cho cây 
void numbering(node* root)
{
    queue<node*> q;
    int v=256;
    q.push(root);

    while(!q.empty())
    {
        int sz=q.size();
        while(sz--)
        {
            node* t=q.front();
            q.pop();
            // gán node number cho node hiện tại
            t->node_number=v;
            v--; // decrease value

            if(t->right!=NULL)
            {
                q.push(t->right);
                q.push(t->left);
            }
        }
    }
}

// Hàm in cây huffman
void print(node* root)
{
    queue<node*> q;
    q.push(root);

    while(!q.empty())
    {
        int sz=q.size();
        while(sz--)
        {
            node* t=q.front();
            q.pop();
            cout<<t->c<<" "<<t->value<<" "<<t->node_number<<" "<<t->isNYT<<endl;
            if(t->left!=NULL)
                q.push(t->left);
            if(t->right!=NULL)
                q.push(t->right);
        }
        cout<<endl;
    }
}

// Hàm Encoder, mã hóa chuỗi s với tham số e và r, từ vị trí start đến end
string Encoder(string &s, int &e, int &r, int start, int end)
{
    string encoded_string;
    node* root = new node;
    root->node_number=256;
    root->value=0;
    root->c='\0';
    root->isNYT=true;
    root->left=NULL;
    root->right=NULL;

    vector<bool> m(128,false);

    int i;

    for(i=start;i<=end;i++)
    {
        string temp;
        
        if(m[s[i]-'\0']==true)
        {
            int isPresent=find(root,s[i],temp);
            //add_freq(root,s[i]);
            //build(root);
            //swap(root);
            //numbering(root);
            //temp.clear();
            //int zz=find(root,s[i],temp);
            reverse(temp.begin(),temp.end());
            encoded_string+=temp;
        }
        else
        {
            temp.clear();
            int z=NYTCode(root,temp,s[i]);
            //build(root);
            //swap(root);
            //numbering(root);
        
            reverse(temp.begin(),temp.end());
            string fixed_code=code(temp,e,r,s[i]);
            encoded_string+=fixed_code;
        }
        m[s[i]-'\0']=true;
        //cout<<encoded_string<<endl;
    }

    m.clear();
    //print(root);

    return encoded_string;
}

// Hàm tìm lá tương ứng với mã huffman trong quá trình giải mã, đồng thời update cây
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
            //cout<<r->c<<endl;
            //tnode=root->right;
            //cout<<tnode->c<<endl;

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

// Hàm decoder, giải mã chuỗi bit encoded thành chuỗi ký tự gốc
string Decoder(string &encoded, int &e, int &r)
{
    string decoded;
    //vector<bool> m(128,false);
    node* root = new node;
    root->node_number=256;
    root->value=0;
    root->c='\0';
    root->isNYT=true;
    root->left=NULL;
    root->right=NULL;

    int i=0;
    int n=encoded.length();

    char dec;
    node* tnode = root;

    while(i<n)
    {
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
            //string temp;
            // int z=NYTCode(root,temp,nw);
            //cout<<tnode->c<<endl;
            tnode->c=nw;
            //build(root);
            //swap(root);
            //numbering(root);
        }
        else
        {
            decoded+=dec;
            //add_freq(root,dec);
            //build(root);
            //swap(root);
            //numbering(root);
            
        }
        //m[dec-'\0']=true;
        
    }
    
    return decoded;
}

int main(int argc, char** argv) 
{
    MPI_Init(&argc, &argv);
    
    int my_rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    
    string input_text;
    
    // Only rank 0 handles input/output
    if (my_rank == 0) {
        cout << "Using " << nprocs << " MPI processes" << endl;
        
        // Kiểm tra tham số dòng lệnh
        string filename;
        
        if (argc > 1) {
            filename = "../Tests/" + string(argv[1]) + ".txt";
        } else {
            filename = "../Tests/test.txt";
        }
        
        cout << "Processing file: " << filename << endl;
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                input_text += line;
            }
            file.close();
        } else {
            // Nếu không đọc được file, dùng test string mặc định
            input_text = "hello world hello hello";
            cout << "Could not open file, using default test string" << endl;
        }
        
        cout << "Input length: " << input_text.length() << endl;
        cout << "Original string: \"" << input_text << "\"" << endl;
    }
    
    // Broadcast string length to all processes
    int text_length;
    if (my_rank == 0) {
        text_length = input_text.length();
    }
    MPI_Bcast(&text_length, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Broadcast string content to all processes
    if (my_rank != 0) {
        input_text.resize(text_length);
    }
    MPI_Bcast(&input_text[0], text_length, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    int e = 6, r = 64;
    
    // Tính phân đoạn cho mỗi process
    int part = text_length / nprocs;
    int start = part * my_rank;
    int end;
    
    if (my_rank == nprocs - 1) {
        end = text_length - 1;
    } else {
        end = (my_rank + 1) * part - 1;
    }
    
    // Đo thời gian encoding
    double begin_encode, end_encode;
    if (my_rank == 0) {
        begin_encode = omp_get_wtime();
    }
    
    // Mỗi process encode phần của mình
    string local_encoded = Encoder(input_text, e, r, start, end);
    
    // Gather tất cả encoded strings về rank 0
    vector<string> all_encoded(nprocs);
    vector<int> encoded_lengths(nprocs);
    int local_length = local_encoded.length();
    
    MPI_Gather(&local_length, 1, MPI_INT, &encoded_lengths[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (my_rank == 0) {
        all_encoded[0] = local_encoded;
        for (int i = 1; i < nprocs; i++) {
            string temp(encoded_lengths[i], ' ');
            MPI_Recv(&temp[0], encoded_lengths[i], MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            all_encoded[i] = temp;
        }
        end_encode = omp_get_wtime();
    } else {
        MPI_Send(&local_encoded[0], local_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    
    // Rank 0 thực hiện decoding và hiển thị kết quả
    if (my_rank == 0) {
        // Tính tổng độ dài encoded
        int total_encoded_length = 0;
        string full_encoded = "";
        for (int i = 0; i < nprocs; i++) {
            total_encoded_length += all_encoded[i].length();
            full_encoded += all_encoded[i];
        }
        
        cout << "Encoded Length: " << total_encoded_length << endl;
        cout << "Encoded string: " << full_encoded << endl;
        cout << "Encoder Time: " << (end_encode - begin_encode) << " seconds" << endl;
        
        // Đo thời gian decoding
        double begin_decode = omp_get_wtime();
        
        // Decode từng phần và ghép lại
        string decoded_text = "";
        for (int i = 0; i < nprocs; i++) {
            string part_decoded = Decoder(all_encoded[i], e, r);
            decoded_text += part_decoded;
        }
        
        double end_decode = omp_get_wtime();
        
        cout << "Decoded Length: " << decoded_text.length() << endl;
        cout << "Decoded string: \"" << decoded_text << "\"" << endl;
        cout << "Decoder Time: " << (end_decode - begin_decode) << " seconds" << endl;
        
        // Kiểm tra tính đúng đắn
        if (input_text == decoded_text) {
            cout << "Message Decoded Successfully" << endl;
            float compression_ratio = (float)(input_text.length() * 8) / total_encoded_length;
            cout << "Compression Ratio: " << compression_ratio << endl;
        } else {
            cout << "Decoding Failed!" << endl;
        }
    }
    
    MPI_Finalize();
    return 0;
}

/**
// encode 
nếu ký tự đã xuất hiện 
tìm đường đi 0/1 từ root -> node ký tự đó
tăng weight của node ký tự 
xây cây 
hoán đổi nếu cần thiết

nếu ký tự mới
duyệt để tìm NYT node và thêm con
xây cây 
swap nếu cần thiết

//decoder
duyệt cây theo từng bit 
    nếu gặp NYT node 
        đọc e bit đầu và tính ký tự mới thêm vào cây
    else
        đọc ký tự và lặp lại
*/  