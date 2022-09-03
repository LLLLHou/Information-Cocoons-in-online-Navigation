#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

FILE *fstats;

#define CYCLE 100
#define MaxL 5

// Steam
//#define N 87626
//#define M 10978
//#define TB 5094082

// Yelp
//#define N 366715
//#define M 61184
//#define TB 1569264

// Epinions
#define N 39588
#define M 61273
#define TB 586359

// MovieLens
//#define N 247753
//#define M 33670
//#define TB 22884377

char pre_path[150] = "/***";
char dst[20] = "epinions";

int obj[M][MaxL];

int usr_deg[N];
int obj_deg[M];
int in_deg[M];
int out_deg[M];
int usr_status[M];

int o, c, part;
int cyc, rcyc;
int alpha;

int ls[M];
int tmp_ls[M];
int found[M];
long double sim[M];
int INDEX[M];
double ret[1001] = {0.0};
double acc[1001];

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
    strcat(file, "_retrieval.txt");
    fstats = fopen(file, "w");
    
    return;
}

// --------------------------------------------------------------- //

void read_bipartite_network(float rat)
{
    printf("... Reading Bipartite Network\n");
    float pro;
    int u,o,k;
    
    for (u=0;u<N;u++)
    {
        usr_deg[u] = 0;
        usr[u] = (struct neighbour *)malloc(sizeof(struct neighbour));
        usr_t[u] = (struct neighbour *)malloc(sizeof(struct neighbour));
        usr[u] = NULL;
        usr_t[u] = NULL;
        pro = (arc4random()%10000)/(float)10000;
        if (pro<rat)
            usr_status[u] = 1; // trainning user
        else
            usr_status[u] = 0; // testing user
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
        
        if (usr_status[u] == 1)
        {
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
    part = (int)(M/(double)30)+1;
    printf("... ... ");
    common_neighbour();
    
    return;
}

// --------------------------------------------------------------- //

void information_retrieval()
{
    int u, t, k, i;
    int rt;
    int usr_count = 0;
    int cur, cy;
    for (t=0; t<1001; t++)
        acc[t] = 0.0;
    
    for (u=0; u<N; u++)
    {
        if (usr_status[u] == 0 && usr_deg[u] > 1)
        {
            usr_count ++; int count = 0;
            for (o = 0; o<M; o++)
                ls[o] = 0;
            p = usr[u];
            while (p!=NULL)
            {
                i = p->id;
                ls[i] = 1;
                tmp_ls[count] = i;
                count ++;
                p = p->next;
            }
            for (cy = 0; cy < 5*(usr_deg[u]-1); cy ++)
            {
                cur = tmp_ls[arc4random()%usr_deg[u]];
                t = 0; rt = 0;
                for (i=0;i<M;i++)
                    found[i] = 0;
                found[cur] = 1;
                while (t < 1001 && rt < (usr_deg[u]-1))
                {
                    t ++;
                    k = arc4random()%out_deg[cur];
                    k = obj[cur][k];
                    if (ls[k] == 1 && found[k] == 0)
                    {
                        rt ++;  found[k] = 1;
                    }
                    acc[t] += rt/(double)(usr_deg[u]-1);
                    cur = k;
                }
                while (t<1001){
                    t ++;
                    acc[t] += rt/(double)(5*(usr_deg[u]-1)*(usr_deg[u]-1));
                }
                
            }
        }
    }
    for (t=0; t<1001; t++)
        acc[t] = acc[t]/(double)usr_count;
    
    return;
}

// --------------------------------------------------------------- //

void deg_stats()
{
    int i, j;
    for (i=0; i<M; i++){
        in_deg[i] = 0;
        out_deg[i] = 0;
    }
    for (i=0; i<M; i++){
        for (j=0; j<MaxL; j++){
            if (obj[i][j] != -1){
                out_deg[i] ++;
                in_deg[obj[i][j]] ++;
            }
        }
    }
    
    return;
}

// --------------------------------------------------------------- //

int main()
{
    file_df();
    for (o=0; o<1001; o++)
        ret[o] = 0.0;
    for (cyc=0; cyc<CYCLE; cyc++)
    {
        printf("\n ... cycle: %d", cyc);
        read_bipartite_network(0.9);
        recommendation_network_construction();
        deg_stats();
        information_retrieval();
        printf("\t\t%.4f\n", acc[50]);
        for (o=0; o<1001; o++)
            ret[o] += acc[o]/(double)CYCLE;
    }
    for (o=0; o<1001; o++)
        fprintf(fstats, "%d\t%f\n", o, ret[o]);
    fclose(fstats);
    
    return 0;
}
