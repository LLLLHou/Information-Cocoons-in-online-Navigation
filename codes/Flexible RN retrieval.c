#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

FILE *fstats;

#define MaxL 1000
#define CYCLE 10
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
int obj[M][MaxL];

int usr_deg[N];
int obj_deg[M];
int in_deg[M];
int out_deg[M];
int usr_status[N];

int o, c, part;
int cyc, dcycle;
int alpha;

long double sim[M];
int ls[M];
int tmp_ls[M];
int found[M];
int INDEX[M];

double ret5, ret10, ret20, ret50, ret100, ret200;

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

void information_retrieval()
{
    int u, t, k, i;
    int rt;
    int usr_count = 0;
    int cur, cy;
    
    part = (int)(N/(double)30)+1;
    ret5 = 0.0; ret10 = 0.0; ret20 = 0.0; ret50 = 0.0; ret100 = 0.0; ret200 = 0.0;
    
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
                while (t < 200)
                {
                    t ++;
                    k = arc4random()%out_deg[cur];
                    k = obj[cur][k];
                    if (ls[k] == 1 && found[k] == 0)
                    {
                        rt ++;  found[k] = 1;
                    }
                    if (t==5){
                        ret5 += rt/(double)(5*(usr_deg[u]-1)*(usr_deg[u]-1));
                    }
                    else if (t==10){
                        ret10 += rt/(double)(5*(usr_deg[u]-1)*(usr_deg[u]-1));
                    }
                    else if (t==20){
                        ret20 += rt/(double)(5*(usr_deg[u]-1)*(usr_deg[u]-1));
                    }
                    else if (t == 50){
                        ret50 += rt/(double)(5*(usr_deg[u]-1)*(usr_deg[u]-1));
                    }
                    else if (t == 100){
                        ret100 += rt/(double)(5*(usr_deg[u]-1)*(usr_deg[u]-1));
                    }
                    cur = k;
                }
                ret200 += rt/(double)(usr_deg[u]-1);
            }
        }
    }
    ret5 = ret5/(double)(usr_count);
    ret10 = ret10/(double)(usr_count);
    ret20 = ret20/(double)(usr_count);
    ret50 = ret50/(double)(usr_count);
    ret100 = ret100/(double)(usr_count);
    ret200 = ret200/(double)(usr_count);
    
    return;
}

// --------------------------------------------------------------- //

int main()
{
    file_df();
    for (dcycle=0; dcycle<10; dcycle++){
        read_bipartite_network(0.9);
        recommendation_network_construction();
        for (alpha=0; alpha<29; alpha++){
            printf("\n... L=%d ...", SL[alpha]);
            for (cyc=0; cyc<CYCLE; cyc++){
                make_random_reconet();
                information_retrieval();
                printf("%f %f\n", ret10, ret50);
                fprintf(fstats, "%d\t%f\t%f\t%f\t%f\t%f\t%f\n", SL[alpha], ret5, ret10, ret20, ret50, ret100, ret200);
            }
        }
    }
    
    fclose(fstats);
    
    return 0;
}
