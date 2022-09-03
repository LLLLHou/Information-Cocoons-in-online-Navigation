#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

FILE *fstats;

#define MaxL 1000
#define CYCLE 100
#define L 5

// Steam
//#define N 87626
//#define M 10978
//#define TB 5094082

// Yelp
//#define N 366715
//#define M 61184
//#define TB 1569264

// Epinions
//#define N 40163
//#define M 139738
//#define TB 664824

// MovieLens
#define N 247753
#define M 33670
#define TB 22884377

char pre_path[150] = "/***";
char dst[20] = "movielens";

int FUNET[M][MaxL];
int obj[M][L];

int usr_deg[N];
int obj_deg[M];
int in_deg[M];
int out_deg[M];
int trap_node[M];

int o, c, part;
int cyc;
int alpha;

long double sim[M];
int ls[M];
int tmp_ls[M];
int found[M];

int tpls[M][100];
int tpsz[M];
int tpN;
int trap_count;
int INDEX[M];
int freq[M];
double navig;

int SL[29] = {5, 6, 8, 9, 11, 12, 15, 17, 20, 24, 28, 33, 39, 46, 54, 63, 74, 87, 102, 119, 140, 164, 192, 226, 264, 310, 363, 426, 500};

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
    strcat(file, "_navigability.txt");
    fstats = fopen(file, "w");
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
        FUNET[o][k] = INDEX[k];
        k ++;
    }
    if (k==0){
        k = o;
        while (k==o)
            k = arc4random()%M;
        FUNET[o][0] = k;
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
            FUNET[o][i] = -1;
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
        
        for (j=0; j<M; j++)
        {
            sim[j] += (arc4random()%100000)/100000;
        }
        
        ranking_sim();
    }
    
    return;
}

// --------------------------------------------------------------- //

void recommendation_network_construction()
{
    part = (int)(M/(double)60)+1;
    common_neighbour();
    
    return;
}

// --------------------------------------------------------------- //

double random_walk(int ot)
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
        freq[ot] ++;
    }
    
    return fd/(double)M;
}

// --------------------------------------------------------------- //

double random_walk_module()
{
    double navigability = 0.0;
    
    part = (int)(M/(double)20)+1;
    printf("\n ... ...");
    for (c=0;c<M;c++)
    {
        if ((c+1)%part == 0){printf("=");}
        o = arc4random()%M;
        navigability += random_walk(o);
    }
    
    return navigability/(double)M;
}

// --------------------------------------------------------------- //

int MIN(int x, int y)
{
    if (x > y)
        return y;
    else
        return x;
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
    
    if (n>MIN(SL[alpha], 5) && n<100)
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
                    for (i=0; i<s; i++){
                        trap_node[tpls[k][i]] = 1;
                    }
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
    tpN = 0;
    for (o=0; o<M; o++)
        tpN += trap_node[o];
    
    return;
}

// --------------------------------------------------------------- //

void make_random_reconet()
{
    int k, ct, j;
    for (o=0;o<M;o++)
        in_deg[o] = 0;
    for (o=0;o<M;o++){
        out_deg[o] = 0;
        for (k=0;k<5;k++)
            obj[o][k] = -1;
        k = 0;
        while (FUNET[o][k] != -1 && k<SL[alpha]){
            ls[k] = FUNET[o][k];
            k++;
        }
        ct = k;
        if (ct<=5){
            for (k=0; k<ct; k++){
                obj[o][k] = ls[k];
                out_deg[o] ++;
                in_deg[ls[k]] ++;
            }
        }
        else{
            for (k=0; k<5; k++){
                j = -1;
                while(j == -1 | ls[j] == -1)
                    j = arc4random()%ct;
                obj[o][k] = ls[j];
                out_deg[o] ++;
                in_deg[ls[j]] ++;
                ls[j] = -1;
            }
        }
    }
    
    return;
}

// --------------------------------------------------------------- //

int main()
{
    file_df();
    read_bipartite_network();
    recommendation_network_construction();
    for (alpha=0; alpha<29; alpha++){
        printf("\n... L=%d ...\n", SL[alpha]);
        for (cyc=0; cyc<CYCLE; cyc++){
            make_random_reconet();
            navig = random_walk_module();
            trap_module();
            printf("%f\t%d\t%d", navig, trap_count, tpN);
            fprintf(fstats, "%d\t%f\t%d\t%d\n", SL[alpha], navig, trap_count, tpN);
        }
    }
    
    fclose(fstats);
    
    return 0;
}
