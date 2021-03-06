#include "head.h"
#include "tools.h"
#include "print.h"
#include "read.h"
#include "parallel.h"

void read_headfile(char *headfile)
{
    char str[BUFF] = "0";
    int i;
    Nfile = 0;
    FILE *ifp = fopen(headfile, "r");
    while(fscanf(ifp, "%s",str)==1){
	fprintf(stderr, "  %s", str);
	if(access(str, R_OK) == -1) {fprintf(stderr, " does not exist!\n"); exit(-1);}
	fprintf(stderr, "\n");
	Nfile++;}
    fclose(ifp);
    ibdfile = (char **)calloc(Nfile, sizeof(char*));
    memSize += Nfile * sizeof(char*) + sizeof(char**);

    ifp = fopen(headfile, "r"); i = 0;
    while(fscanf(ifp, "%s",str)==1){
	ibdfile[i] = (char * )calloc(strlen(str)+10, sizeof(char));
	memSize += (strlen(str)+10) * sizeof(char);
	strcpy(ibdfile[i], str);

	i ++;
    }
    fclose(ifp);
}

int check_exist(char *str)
{
    char *inp;
    inp = (char *) calloc (strlen(str)+10, sizeof(char));
    sprintf(inp, "%s", str);
    char delim[] = ",";

    char *ptr = strtok(inp, delim);
    while(ptr != NULL)
    {
	if(access(ptr, R_OK) == -1) {fprintf(stderr, "\n%s", ptr); return -1;}
	ptr = strtok(NULL, delim);
    }
    free(inp);
    return 0;
}

int check_input(int arg, char**argv)
{
    fprintf(stderr, "############################\n");
    fprintf(stderr, "#Welcome to IBDkin V2.8.7.3#\n");
    fprintf(stderr, "############################\n");
    fprintf(stderr, "Copyright (C) 2020 Ying Zhou\n\n");
    //fprintf(stderr, "zlib verison: %s\n\n", ZLIB_VERSION);
    int i, fsize;
    IBDcM = 0;
    checkTime = 0; checkMem = 0;
    BINSIZE = 1000; FOLD = 4;
    IBDcM = 4; IBDcM2 = 2; degree = 9; gap1 = 5; gap2 = 20;
    kincut = pow(2, -degree - 1.5);
    Parts = 1; part = 1;
    tagKinship = 1;
    tagMask = 0;
    tagCoverage = 0;
    Nthreads = 2;
    outfile = (char *) calloc (16, sizeof(char));
    maskfile = (char *) calloc (16, sizeof(char));
    coveragefile = (char *) calloc (16, sizeof(char));
    memSize += 3* 16 * sizeof(char);
    strcpy(outfile, "kinship.gz");
    strcpy(maskfile, "mask.gz");
    strcpy(coveragefile, "coverage.gz");



    if(arg <=1)print_help();

    for(i = 1; i< arg; i++){
	if(strcmp(argv[i], "-ibdfile")==0){
	    fsize = strlen(argv[i+1])+10;
	    headfile = (char *) calloc (fsize, sizeof(char));
	    memSize += (fsize) * sizeof(char);
	    strcpy(headfile, argv[i+1]); i = i + 1;
	}
	else if(strcmp(argv[i], "-map")==0){
	    fsize = strlen(argv[i+1])+10;
	    mapfile = (char *) calloc (fsize, sizeof(char));
	    memSize += (fsize) * sizeof(char);
	    strcpy(mapfile, argv[i+1]); i = i + 1;
	}
	else if(strcmp(argv[i], "-ind")==0){
	  fsize = strlen(argv[i+1])+10;
	    idfile = (char *) calloc (fsize, sizeof(char));
	    memSize += (fsize) * sizeof(char);
	    strcpy(idfile, argv[i+1]); 
	    i = i + 1;
	}
	else if(strcmp(argv[i], "-out")==0){
	    fsize = strlen(argv[i+1])+20;
            outfile = realloc (outfile, fsize*sizeof(char));
            maskfile = realloc (maskfile, fsize*sizeof(char));
            coveragefile = realloc (coveragefile, fsize*sizeof(char));
            memSize += 3* (fsize - 16) * sizeof(char);
            sprintf(outfile, "%s.kinship.gz", argv[i+1]); 
            sprintf(maskfile, "%s.mask.gz", argv[i+1]); 
            sprintf(coveragefile, "%s.coverage.gz", argv[i+1]); 
	    i = i + 1;
        }

	else if(strcmp(argv[i], "-cutcm")==0){
	    IBDcM = atof(argv[i+1]);
	    IBDcM2 = atof(argv[i+2]);
	    i = i + 2;
	}
	else if(strcmp(argv[i], "-binkb")==0){
            BINSIZE = atof(argv[i+1]);i = i + 1;
        }
	else if(strcmp(argv[i], "-fold")==0){
            FOLD = atof(argv[i+1]);i = i + 1;
        }
	else if(strcmp(argv[i], "-degree")==0){
	    degree = atoi(argv[i+1]);i = i + 1;
	    kincut = pow(2, -degree - 1.5);
	}
	else if(strcmp(argv[i], "-nthreads")==0){
	    Nthreads = atoi(argv[i+1]);i = i + 1;
	}
	else if(strcmp(argv[i], "--checkTime")==0){
	    checkTime = 1;
	}
	else if(strcmp(argv[i], "--checkMem")==0){
            checkMem = 1;
        }
	else if(strcmp(argv[i], "--nokinship")==0){
	    tagKinship = 0;
	}
	else if(strcmp(argv[i], "--outmask")==0){
	    tagMask = 1;
	}
	else if(strcmp(argv[i], "--outcoverage")==0){
	    tagCoverage = 1;
	}
	else if(strcmp(argv[i], "-part")==0){
	    Parts = atoi(argv[i+1]);
	    part = atoi(argv[i+2]);
	    i = i + 2;
	}
	else if(strcmp(argv[i], "-merge")==0){
	    gap1 = atof(argv[i+1]);
	    gap2 = atof(argv[i+2]);
	    i = i + 2;
	}

	else {
	    print_help();
	}
    }
    if( tagKinship + tagMask + tagCoverage == 0 ){
	fprintf(stderr, "No output is specified!\n");
	print_help();
    }

    return 0;
}


void write_relate(void)
{
    double a = my_wallclock();
    long int b = clock();
    gzFile fp = gzopen(outfile, "a");
    gzbuffer(fp, BUFF3);
    Pair_t *prcur;
    int i, j, idi, ids, segnum;
    float kinship, s0, s1, s2;
    char str[BUFF];
    for(i = 0; i < idNum; i++){
	for(j = 0; j < idhead[i]->num; j++){
	    prcur = (idhead[i]->head)[j];
	    idi = i;
	    ids = prcur->index;
	    segnum = prcur->segnum;;
	    s1 = prcur->s1 / totg;
	    s2 = prcur->s2 / totg;
	    s0 = 1 - s1 - s2;
	    kinship = s1/4+s2/2;
	    if(kinship >= kincut){
		if(s2 <= 0.00001)sprintf(str, "%s %s %d %f %f 0 %f %d\n", idhead[idi]->id, idhead[ids]->id, segnum, s0, s1, kinship, cal_degree(kinship));
		else sprintf(str, "%s %s %d %f %f %f %f %d\n", idhead[idi]->id, idhead[ids]->id, segnum, s0, s1, s2, kinship, cal_degree(kinship));
		gzputs(fp, str);
	    }
	}
    }
    gzclose(fp);
    Tgzputs += clock() - b;
    t_gzputs += my_wallclock() - a;
    return ;
}

void read_ibd_1st_pass(void)
{
    double a;
    float maskL;
    Round = 0;
    if(checkTime == 1 )a = my_wallclock();


    int i, j;
    for ( i = 1; i < 23; i++ ){
	minPos[i] = INT_MAX; //bp
	maxPos[i] = 0; //bp
    }
    for (i = 0; i < Nfile; i++){
	ifp = gzopen(ibdfile[i], "r");
	gzbuffer(ifp, BUFF3);
	fprintf(stderr, "\treading %s\n", ibdfile[i]);
	read_buff();
	while(1){
	    Round += 1;
//fprintf(stderr, "round = %d\n", Round);
	    if(buffi2 == 0)break;
	    buffi = buffi2;
//fprintf(stderr, "copy_buff()\n");
	    copy_buff();
//fprintf(stderr, "fill_buff_ibd_1st_pass()\n");
	    fill_buff_ibd_1st_pass();
	    //p_buff();
//fprintf(stderr, "store_buff_ibd()\n");
	    store_buff_ibd();
	}
	gzclose(ifp);
    }
    //p_datamap();
    //p_pair_depth();
    //cal_pair_num();

    cal_coverage_median();
    create_mask();

    fprintf(stderr, "\n  IBD coverages:\n");
    totg = 0; maskL = 0;
    for ( i = 1; i < 23; i++ ){
	for (j = 0; j < Nthreads; j++){
	    if(minPos[i] > minpos[i][j]) minPos[i] = minpos[i][j];
	    if(maxPos[i] < maxpos[i][j]) maxPos[i] = maxpos[i][j];
	}
	if( minPos[i] < maxPos[i]){
	    minPos[i] = interpolate2cM(i, minPos[i]);
	    maxPos[i] = interpolate2cM(i, maxPos[i]);
	    //totg += maxPos[i] - minPos[i];
	    maskL = apply_mask(i, minPos[i], maxPos[i]);
	    totg += maxPos[i] - minPos[i] - maskL;
	    fprintf(stderr, "\tchr%d: %.2f -> %.2f cM, masked length = %.2fcM\n",i, minPos[i], maxPos[i], maskL);
	}

    }
    fprintf(stderr, "  Total chromosome length: %f cM\n", totg);
    if(checkTime ==1 ){
	p_std_time(my_wallclock() - a);
	fprintf(stderr,"\nread_ibd_1st_pass() Time = %s\n", timestr);}
    return ;
}


void read_ibd_2nd_pass(void)
{
    double a;
    if(checkTime)a = my_wallclock();
    int i;
    Round = 0;

    //ofp = fopen(outfile, "a");
    //gzFile ifp;
    for (i = 0; i < Nfile; i++){
	ifp = gzopen(ibdfile[i], "r");
	fprintf(stderr, "\treading %s\n", ibdfile[i]);
	read_buff();
	sscanf(strbuff2[i], "%*s %*s %*s %*s %d", &CHR);
	while(1){
	    Round += 1;
	    if(buffi2 == 0)break;
	    buffi = buffi2;
	    copy_buff();
	    fill_buff_ibd_2nd_pass();
	    //p_buff();
	    cal_kinship();//for distant relatives
	    write_buff();
	}
	cal_pair_num();
	p_mem();
	cal_kinship2();//for close relatives
	gzclose(ifp);
    }
    //fclose(ofp);
    write_relate();

    //p_datamap();
    if(checkTime){
	p_std_time(my_wallclock() - a);
	fprintf(stderr,"read_ibd_2nd_pass() Time = %s\n", timestr);
    }
}




void read_map(char *plinkmap)
{
    double a = my_wallclock();
    FILE *ifp;
    int i, chr;
    char str[BUFF];
    float p, g;
    int mapi[23];
    for(i = 0; i <=22; i++)mapL[i]=0;
    ifp=fopen(plinkmap, "r");
    while(fgets(str, BUFF-1, ifp)!=NULL){
	chr = -1;
	if(sscanf(str, "%d", &chr) == 1)mapL[chr] ++;
    }
    fclose(ifp);

    mapP = (float **)calloc(23, sizeof(float*));
    mapG = (float **)calloc(23, sizeof(float*));
    memSize += 2*23*sizeof(float*);
    for(i = 1; i<= 22; i++){
	mapP[i] = (float *)calloc(mapL[i], sizeof(float));
	mapG[i] = (float *)calloc(mapL[i], sizeof(float));
	memSize += 2*mapL[i]*sizeof(float);
    }
    ifp=fopen(plinkmap, "r");
    i=0;
    for(chr = 1; chr <=22; chr++)mapi[chr]=0;
    while(fscanf(ifp, "%d %*s %f %f", &chr, &g, &p)==3){
	mapG[chr][mapi[chr]] = g;
	mapP[chr][mapi[chr]] = p;
	mapi[chr] ++;
    }
    fclose(ifp);
    /*check input map*/
    for (chr = 1; chr <=22; chr ++){
	for(i=0;i<(mapL[chr]-1);i++){
	    if(mapP[chr][i+1]<mapP[chr][i]||mapG[chr][i+1]<mapG[chr][i]){
		fprintf(stderr, "Error: map position should be in increasing order!\n error in line %d %.0f %.0f\n",chr, mapP[chr][i], mapP[chr][i+1]);
		exit(-1);
	    }
	}
    }
    p_std_time(my_wallclock() - a);
    fprintf(stderr,"read_genetic_map() Time = %s\n", timestr);
    return ;
}

void read_ind(char *idfile)
{
    double a = my_wallclock();
    FILE *ifp;
    int i;
    char str[BUFF];
    ifp=fopen(idfile, "r"); idNum = 0;
    while(fscanf(ifp, "%s", str)==1)idNum ++;
    fclose(ifp);
    idhead = (ID_t **)calloc(idNum, sizeof(ID_t*));
    lock = (char *)calloc(idNum, sizeof(char));
    memSize += idNum * (sizeof(ID_t*) + sizeof(char)) + sizeof(ID_t**);
    ifp=fopen(idfile, "r"); i = 0;
    while(fscanf(ifp, "%s", str)==1){
	idhead[i] = (ID_t *)calloc(1, sizeof(ID_t));
	idhead[i]->id = (char *)calloc(strlen(str)+1,sizeof(char));
	idhead[i]->head = NULL;
	idhead[i]->num = 0; 
	idhead[i]->stack = (int *)calloc(4, sizeof(int));
	(idhead[i]->stack)[0] = 4;
	memSize += sizeof(ID_t) + strlen(str)*sizeof(char) + 4 * sizeof(int);
	strcpy(idhead[i]->id, str);
	i++;
    }
    fclose(ifp);
    qsort(idhead, idNum, sizeof(ID_t *), ID_t_cmp);

    for(i = 0; i < (idNum-1); i++){
	if(strcmp(idhead[i]->id, idhead[i+1]->id)==0){
	    fprintf(stderr, "duplicated individuals: %s\n", idhead[i]->id);
	    exit(-1);
	}
    }

    fprintf(stderr, "%d individuals\n", idNum);
    p_std_time(my_wallclock() - a);
    fprintf(stderr,"read_ind() Time = %s\n", timestr);
    return ;
}


void init(void)
{
    int i, j, chr;
    Tfill_buff_ibd1 =0;
    Tfill_buff_ibd2 =0;
    Tstore_buff_ibd = 0;
    Tcal_kinship = 0;
    Tgzread = 0;
    Tgzputs = 0;
    Tshrinke_id_pair = 0;
    Tcal_pair_num = 0;
    t_fill_buff_ibd1 =0;
    t_fill_buff_ibd2 =0;
    t_store_buff_ibd = 0;
    t_cal_kinship = 0;
    t_gzread = 0;
    t_gzputs = 0;
    t_shrinke_id_pair = 0;
    t_cal_pair_num = 0;

    haptag[0] = '1'; haptag[1] = '2';

    minpos = (float **)calloc(23, sizeof(float *));
    maxpos = (float **)calloc(23, sizeof(float *));
    for(i = 0; i <=22; i++){
	minpos[i] = (float *)calloc(Nthreads, sizeof(float));
	maxpos[i] = (float *)calloc(Nthreads, sizeof(float));
	for(j = 0; j < Nthreads; j++){
	    minpos[i][j] = INT_MAX;
	    maxpos[i][j] = -1;
	}
    }
    memSize += 2 * 23 * (sizeof(float *) + Nthreads * sizeof(float));

    fprintf(stderr, "allocate IBD buffer\n");
    IBDdat = (IBD_t **)calloc(BUFF2, sizeof(IBD_t*));
    strbuff = (char **)calloc(BUFF2, sizeof(char*));
    strbuff2 = (char **)calloc(BUFF2, sizeof(char*));
    for(i = 0; i < BUFF2 ;i ++){
	IBDdat[i] = (IBD_t *)calloc(1, sizeof(IBD_t));
	strbuff[i] = (char *)calloc(BUFF, sizeof(char));
	strbuff2[i] = (char *)calloc(BUFF, sizeof(char));
	memSize += sizeof(IBD_t) + 2 * BUFF * sizeof(char);
    }
    memSize += BUFF2 * (sizeof(IBD_t*) + 2 * sizeof(char*));

    float bin = BINSIZE * 1000;
    coverageL[0] = (int) (300000000 / bin) + 1;
    coverageL[1] = (int) (300000000 / bin) + 1;
    coverageL[2] = (int) (300000000 / bin) + 1;
    coverageL[3] = (int) (250000000 / bin) + 1;
    coverageL[4] = (int) (250000000 / bin) + 1;
    coverageL[5] = (int) (250000000 / bin) + 1;
    coverageL[6] = (int) (250000000 / bin) + 1;
    coverageL[7] = (int) (250000000 / bin) + 1;
    coverageL[8] = (int) (200000000 / bin) + 1;
    coverageL[9] = (int) (200000000 / bin) + 1;
    coverageL[10] = (int) (200000000 / bin) + 1;
    coverageL[11] = (int) (200000000 / bin) + 1;
    coverageL[12] = (int) (200000000 / bin) + 1;
    coverageL[13] = (int) (150000000 / bin) + 1;
    coverageL[14] = (int) (150000000 / bin) + 1;
    coverageL[15] = (int) (150000000 / bin) + 1;
    coverageL[16] = (int) (150000000 / bin) + 1;
    coverageL[17] = (int) (100000000 / bin) + 1;
    coverageL[18] = (int) (100000000 / bin) + 1;
    coverageL[19] = (int) (100000000 / bin) + 1;
    coverageL[20] = (int) (100000000 / bin) + 1;
    coverageL[21] = (int) (100000000 / bin) + 1;
    coverageL[22] = (int) (100000000 / bin) + 1;

    /*the final coverage file*/
    coverage = (double **)calloc(23, sizeof(double*));
    memSize += sizeof(double **);
    for(chr = 0; chr < 23; chr++){
	coverage[chr] = (double *)calloc(coverageL[chr], sizeof(double));
	memSize += sizeof(double *) + coverageL[chr] * sizeof(double);
	for(i = 0; i < coverageL[chr]; i ++){
	    coverage[chr][i] = -0.00000001;
	}
    }

    /*store coverage for each thread*/
    double **tf;
    coverage_per_thread = (cvrg_t **)calloc(Nthreads, sizeof(cvrg_t *));
    memSize += sizeof(cvrg_t **);
    for (i = 0; i < Nthreads; i++){
	coverage_per_thread[i] = (cvrg_t *)calloc(1, sizeof(cvrg_t));
	tf = (double **)calloc(23, sizeof(double*));
	memSize += sizeof(cvrg_t) + 23 * sizeof(double*);
	for(chr = 0; chr < 23; chr++){
	    tf[chr] = (double *)calloc(coverageL[chr], sizeof(double));
	    memSize += coverageL[chr] * sizeof(double);
	    for(j = 0; j < coverageL[chr]; j ++){
		tf[chr][j] = -0.00000001;
	    }
	}
	coverage_per_thread[i]->coverage = tf;
    }

    mask = (msk_t **)calloc(23, sizeof(msk_t *));
    memSize += sizeof(msk_t **);
    for(chr = 0; chr < 23; chr++){
	mask[chr] = (msk_t *)calloc(1, sizeof(msk_t));
	memSize += sizeof(msk_t);
	mask[chr]->p1 = -1;
	mask[chr]->p2 = -1;
	mask[chr]->g1 = -1;
	mask[chr]->g2 = -1;
	mask[chr]->next = NULL;
    }

    return ;
}


