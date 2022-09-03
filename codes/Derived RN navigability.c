#include<stdio.h>
#include<stdlib.h>
#include<string.h>

FILE *fnodes;
FILE *ftraps;
FILE *fnavig;

#define MaxL 5

// Steam
#define N 87626
#define M 10978
#define TB 5094082

// Yelp
//#define N 366715
//#define M 61184
//#define TB 1569264

// Epinions
//#define N 40163
//#define M 139738
//#define TB 664824

// MovieLens
//#define N 247753
//#define M 33670
//#define TB 22884377

char pre_path[150] = "/***";
char dst[20] = "steam";

int in_deg[M];
int obj[M][MaxL];
int out_deg[M];
int usr_deg[N];
int obj_deg[M];

int freq[M];
double frequency[M];
int trap_node[M];
int trap_count;
double navi[M+1];

int c, o, part;
int ls[M], tmp_ls[M], found[M];

int tpls[M][100];
int tpsz[M];
int tpN;

long double sim[M];
int INDEX[M];

// --------------------------------------------------------------- //

struct neighbour
{
    int id;
    struct neighbour *next;
};struct neighbour *usr[N], *usr_t[N], *obj_bip[M], *obj_bip_t[M], *p, *pp;

// --------------------------------------------------------------- //

void file_df()
{
    char file[200];
    strcpy(file, pre_path);
    strcat(file, dst);
    strcat(file, "_nodes.txt");
    fnodes = fopen(file, "w");
    
    strcpy(file, pre_path);
    strcat(file, dst);
    strcat(file, "_trap_list.txt");
    ftraps = fopen(file, "w");
    
    strcpy(file, pre_path);
    strcat(file, dst);
    strcat(file, "_navigability.txt");
    fnavig = fopen(file, "w");
    
    return;
}

// --------------------------------------------------------------- //

void read_bipartite_network()
{
    printf("... Reading Bipartite Network\n");
    int u,o,k;
    
    for (u=0;u<N;u++)
    {
        usr_deg[u] = 0;
        usr[u] = (struct neighbour *)malloc(sizeof(struct neighbour));
        usr_t[u] = (struct neighbour *)malloc(sizeof(struct neighbour));
        usr[u] = NULL;
        usr_t[u] = NULL;
    }
    for (o=0;o<M;o++)
    {
        obj_deg[o] = 0;
        obj_bip[o] = (struct neighbour *)malloc(sizeof(struct neighbour));
        obj_bip_t[o] = (struct neighbour *)malloc(sizeof(struct neighbour));
        obj_bip[o] = NULL;
        obj_bip_t[o] = NULL;
    }
    
    char file[200];
    strcpy(file, pre_path);
    strcat(file, dst);
    strcat(file, ".txt");
    FILE *fp = fopen(file, "r");
    for (k=0;k<TB;k++)
    {
        fscanf(fp,"%d %d\n",&u,&o);
        usr_deg[u] ++;
        p = (struct neighbour *)malloc(sizeof(struct neighbour));
        p->id = o;
        if (usr_deg[u] == 1)
        {
            usr[u] = p;
            usr_t[u] = p;
        }
        else
        {
            usr_t[u]->next = p;
            usr_t[u] = p;
        }
        p -> next = NULL;
        
        obj_deg[o] ++;
        p = (struct neighbour *)malloc(sizeof(struct neighbour));
        p->id = u;
        if (obj_deg[o] == 1)
        {
            obj_bip[o] = p;
            obj_bip_t[o] = p;
        }
        else
        {
            obj_bip_t[o]->next = p;
            obj_bip_t[o] = p;
        }
        p -> next = NULL;
    }
    fclose(fp);
    printf("... ... DONE\n");
    
    return;
}

// --------------------------------------------------------------- //

void Quick_Sort(int left, int right, long double A[])
{
    int i,j;
    long double middle;
    int temp;
    
    i=left;
    j=right;
    middle=A[INDEX[(int)(left+right)/2]];
    do
    {
        while(A[INDEX[i]]>middle && i<right)
            i++;
        while(A[INDEX[j]]<middle && j>left)
            j--;
        if(i<=j)
        {
            temp=INDEX[i];
            INDEX[i]=INDEX[j];
            INDEX[j]=temp;
            i++;
            j--;
        }
    }while(i<=j);
    
    if(left<j)
        Quick_Sort(left,j,A);
    if(right>i)
        Quick_Sort(i,right,A);
    
    return;
}

// --------------------------------------------------------------- //

void ranking_sim()
{
    int left, right;
    int k;
    
    left = 0; right = M-1;
    Quick_Sort(left, right, sim);
    
    k = 0;
    while(sim[INDEX[k]] > 0.0 && k<MaxL){
        obj[o][k] = INDEX[k];
        k ++;
    }
    if (k==0){
        k = o;
        while (k==o)
            k = arc4random()%M;
        obj[o][0] = k;
    }
    
    return;
}

// --------------------------------------------------------------- //

void common_neighbour()
{
    int u, j, i;
    for (o=0; o<M; o++)
    {
        if ((o+1)%part == 0){printf("=");}
        for (i=0; i<MaxL; i++)
            obj[o][i] = -1;
        for (j=0; j<M; j++){
            sim[j] = 0.0;
            INDEX[j] = j;
        }
        p = obj_bip[o];
        while (p!=NULL)
        {
            u = p->id;
            pp = usr[u];
            while (pp!=NULL)
            {
                j = pp->id;
                sim[j] += 1.0;
                pp = pp->next;
            }
            p = p->next;
        }
        sim[o] = -1.0;
        
        ranking_sim();
    }
    
    return;
}

// --------------------------------------------------------------- //

void recommendation_network_construction()
{
    part = (int)(M/(double)40)+1;
    printf("... ... ");
    common_neighbour();
    
    return;
}

// --------------------------------------------------------------- //

void deg_stats()
{
    int i, k;
    for (i=0;i<M;i++){
        out_deg[i] = 0;
        in_deg[i] = 0;
    }
    for (i=0; i<M; i++){
        for (k=0; k<MaxL; k++){
            if (obj[i][k]!=-1){
                out_deg[i] ++;
                in_deg[obj[i][k]] ++;
            }
        }
    }
    
    return;
}

// --------------------------------------------------------------- //

void random_walk(int ot)
{
    int i, to, t;
    int fd = 0;
    
    for (i=0; i<M; i++)
        freq[i] = 0;
    freq[ot] = 1;
    t = 0;
    while (t<=M)
    {
        t ++;
        i = arc4random()%out_deg[ot];
        to = obj[ot][i];
        ot = to;
        if (freq[ot] == 0)
            fd ++;
        navi[t] += fd/(double)M;
        if (t<=M)
            freq[ot] ++;
    }
    
    for (i=0; i<M; i++)
        frequency[i] += freq[i]/(double)(M);
    
    return;
}

// --------------------------------------------------------------- //

void random_walk_module()
{
    int i;
    part = (int)(M/(double)50)+1;
    printf("\n ... ...");
    for (i=0; i<M; i++){
        frequency[i] = 0.0;
    }
    for (c=0;c<M;c++)
    {
        if ((c+1)%part == 0){printf("=");}
        o = arc4random()%M;
        random_walk(o);
    }
    
    return;
}

// --------------------------------------------------------------- //

void finding_trap(int ot)
{
    int n = 1;
    int new, k, i, j;
    for(i=0; i<M; i++)
        found[i] = 0;
    found[ot] = 1;
    ls[0] = ot;
    new = 1;
    while (new>0 && n<100)
    {
        int numb = new;
        new = 0;
        for (k=0; k<numb; k++)
        {
            j = ls[k];
            for (i=0;i<out_deg[j];i++)
            {
                if (found[obj[j][i]] == 0)
                {
                    tmp_ls[new] = obj[j][i];
                    found[obj[j][i]] = 1;
                    new ++;
                    n ++;
                }
            }
        }
        for (i=0;i<new;i++)
            ls[i] = tmp_ls[i];
    }
    
    if (n>MaxL && n<100)
    {
        // found a seemingly trap
        tpsz[tpN] = n;
        k = 0;
        for (i=0;i<M;i++){
            if (found[i] == 1){
                tpls[tpN][k] = i;
                k ++;
            }
        }
        tpN ++;
    }
    
    return;
}

// --------------------------------------------------------------- //

void core_trap()
{
    int s, k, i;
    trap_count = 0;
    for (s=2; s<100; s++){
        for (k=0; k<tpN; k++){
            if (tpsz[k] == s){
                int tt = 1;
                i = 0;
                while (tt==1 && i<s){
                    if (trap_node[tpls[k][i]] == 1)
                        tt = 0;
                    i ++;
                }
                if (tt == 1)
                {
                    trap_count ++;
                    double tf = 0.0;
                    int tdeg = 0;
                    for (i=0; i<s; i++){
                        trap_node[tpls[k][i]] = 1;
                        tf += frequency[tpls[k][i]];
                        tdeg += in_deg[tpls[k][i]];
                    }
                    fprintf(ftraps, "%d %f %f\n", s, tdeg/(double)s, tf/(double)s);
                }
            }
        }
    }
    
    return;
}

// --------------------------------------------------------------- //

void trap_module()
{
    int x;
    for (o=0; o<M; o++){
        trap_node[o] = 0;
        for (x=0; x<100; x++)
            tpls[o][x] = -1;
        tpsz[o] = 0;
    }
    tpN = 0;
    
    for (o=0; o<M; o++)
        if (in_deg[o] > 0)
            finding_trap(o);
    
    core_trap();
    
    return;
}

// --------------------------------------------------------------- //

int main()
{
    read_bipartite_network();
    file_df();
    recommendation_network_construction();
    deg_stats();
    random_walk_module();
    trap_module();
    
    for (o=0; o<M; o++)
        fprintf(fnodes, "%d\t%d\t%d\t%d\t%f\n", o, trap_node[o], in_deg[o], out_deg[o], frequency[o]);
    for (o=1; o<M+1; o++)
        fprintf(fnavig, "%d\t%f\n", o, navi[o]);
    
    fclose(fnodes);fclose(ftraps);fclose(fnavig);
    
    return 0;
}
