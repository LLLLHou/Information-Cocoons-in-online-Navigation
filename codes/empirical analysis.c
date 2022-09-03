#include<stdio.h>
#include<stdlib.h>
#include<string.h>

FILE *fnodes;
FILE *ftraps;
FILE *fnavig;

#define MaxL 5

// science article
#define M 7730
#define T 26338

// pnas article
// #define M 59479
// #define T 261394

// amazon kindle
//#define M 119636
//#define T 584093

char pre_path[150] = "/***";
char dst[20] = "science";

int in_deg[M];
int obj[M][MaxL];
int out_deg[M];

int freq[M];
double frequency[M];
int trap_node[M];
int trap_count;
double navi[10*M+1] = {0.0};

int c, o, part;
int ls[M], tmp_ls[M], found[M];

int tpls[M][100];
int tpsz[M];
int tpN;

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

void read_reco_net()
{
    printf("reading recommendation network\n");
    int i, j, k, f;
    for (i=0; i<M; i++)
        for (k=0; k<MaxL; k++)
            obj[i][k] = -1;
    char file[200];
    strcpy(file, pre_path);
    strcat(file, dst);
    strcat(file, ".txt");
    FILE *fp = fopen(file, "r");
    for (f=0; f<T; f++)
    {
        fscanf(fp, "%d %d %d\n", &i, &j, &k);
        obj[i][k] = j;
    }
    fclose(fp);
    
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
        freq[ot] ++;
    }
    
    for (i=0; i<M; i++)
        frequency[i] += freq[i]/(double)(10*M);
    
    return;
}

// --------------------------------------------------------------- //

void random_walk_module()
{
    int i;
    part = (int)(10*M/(double)50)+1;
    printf("\n ... ...");
    for (i=0; i<M; i++){
        frequency[i] = 0.0;
    }
    for (c=0;c<10*M;c++)
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
    file_df();
    read_reco_net();
    deg_stats();
    random_walk_module();
    trap_module();
    
    for (o=0; o<M; o++)
        fprintf(fnodes, "%d\t%d\t%d\t%d\t%f\n", o, trap_node[o], in_deg[o], out_deg[o], frequency[o]);
    for (o=1; o<10*M+1; o++)
        fprintf(fnavig, "%d\t%f\n", o, navi[o]/(10*M));
    
    fclose(fnodes);fclose(ftraps);fclose(fnavig);
    
    return 0;
}
