#include "my_vm.h"

/*
 * Sets the valid bit to 1 in a bitmap given the index
*/
static void setValid(char *bitmap, int index) {
    // We first find the location in the bitmap array where we want to set a bit
    // Because each character can store 8 bits, using the "index", we find which 
    // location in the character array should we set the bit to.
    char *region = ((char *) bitmap) + (index / 8);
    
    // Now, we cannot just write one bit, but we can only write one character. 
    // So, when we set the bit, we should not distrub other bits. 
    // So, we create a mask and OR with existing values
    char bit = 1 << (index % 8);

    // just set the bit to 1. NOTE: If we want to free a bit (*bitmap_region &= ~bit;)
    *region |= bit;
}

/*
 * Sets the valid bit to 0 in a bitmap given the index
*/
static void setNotValid(char *bitmap, int index) {
    // We first find the location in the bitmap array where we want to set a bit
    // Because each character can store 8 bits, using the "index", we find which 
    // location in the character array should we set the bit to.
    char *region = ((char *) bitmap) + (index / 8);
    
    // Now, we cannot just write one bit, but we can only write one character. 
    // So, when we set the bit, we should not distrub other bits. 
    // So, we create a mask and OR with existing values
    char bit = 1 << (index % 8);

    // just set the bit to 1. NOTE: If we want to free a bit (*bitmap_region &= ~bit;)
    *region &= ~bit;
}

/*
 * Function to get a bit at "index"
*/
static int getBit(char *bitmap, int index) {
    //Same as example 3, get to the location in the character bitmap array
    char *region = ((char *) bitmap) + (index / 8);
    
    //Create a value mask that we are going to check if bit is set or not
    char bit = 1 << (index % 8);
    
    return (int)(*region >> (index % 8)) & 0x1;
}

/*
 * Gets the num_middle_bits bits from num_lower_bits
*/
static unsigned int get_mid_bits (unsigned int value, int num_middle_bits, int num_lower_bits) {

   //value corresponding to middle order bits we will returning.
   unsigned int mid_bits_value = 0;   
    
   // First you need to remove the lower order bits (e.g. PAGE offset bits). 
   value =    value >> num_lower_bits; 


   // Next, you need to build a mask to prune the outer bits. How do we build a mask?   

   // Step1: First, take a power of 2 for â€œnum_middle_bitsâ€  or simply,  a left shift of number 1.  
   // You could try this in your calculator too.
   unsigned int outer_bits_mask =   (1 << num_middle_bits);  

   // Step 2: Now subtract 1, which would set a total of  â€œnum_middle_bitsâ€  to 1 
   outer_bits_mask = outer_bits_mask-1;

   // Now time to get rid of the outer bits too. Because we have already set all the bits corresponding 
   // to middle order bits to 1, simply perform an AND operation. 
   mid_bits_value =  value &  outer_bits_mask;

  return mid_bits_value;

}

/*
 * Gets the top num_bits bits
*/
static unsigned int get_top_bits(unsigned int value,  int num_bits) {
    //Assume you would require just the higher order (outer)  bits, 
    //that is first few bits from a number (e.g., virtual address) 
    //So given an  unsigned int value, to extract just the higher order (outer)  â€œnum_bitsâ€
    int num_bits_to_prune = 32 - num_bits; //32 assuming we are using 32-bit address 
    return (value >> num_bits_to_prune);
}

void* phy_mem = NULL;
void* valid_phy_mem = NULL;

int misses = 0;
int checks = 0;

pthread_mutex_t mutex;
pthread_mutex_t mutex1;

int offset;
int ptBits;
int pdBits;
int phyPages;
int maxPages;
int reservedBits;
int mallocPages;
int PTFlag;
int PTPage;
pte_t maxVA;
/*
 * Simple method to debug the allocated physical memory
*/
void debugPDE(){
    printf("Valid Phy Mem Bitmap\n");
    for(int i = 0; i < maxPages; i++){
        printf("%d", getBit((char*)valid_phy_mem, i));
    }
    printf("\n");

    printf("PDE's\n");
    for(int i = 0; i < pow(2, pdBits); i++){
        for(int j = 31; j >= 0; j--){
            printf("%d", getBit((char*)((pte_t*)phy_mem + i), j));
        }
        printf("\n");
    }
    printf("\n");
    for(int i = 0; i < pow(2, pdBits); i++){
        printf("Index - %d\n", i);
        if(get_top_bits(*((pte_t*)phy_mem+i), 1) == 1){
            void* ptr = (char*)phy_mem + PGSIZE*get_mid_bits(*((pte_t*)phy_mem+i), (ptBits+pdBits), 0);
            for(int j = 0; j < pow(2, ptBits); j++){
                for(int k = 31; k >= 0; k--){
                    printf("%d", getBit((char*)((pte_t*)ptr+j), k));
                }
                printf("\n");
            }
        }
        printf("\n\n");
    }
    printf("\nFirst Index of all phy pages\n");
    for(int i = 0; i < maxPages; i ++){
        printf("Index: %d - %lx\n", i,*(pte_t*)((char*)phy_mem + PGSIZE*i));
    }
    printf("\n");
}

/*
 * Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {
    
    if(phy_mem != NULL || valid_phy_mem != NULL){
        return;
    }
    mallocPages = 0;
    PTFlag = 0;
    PTPage = 0;
    // Calculates the offset, page table, and page directory bits
    offset = (int)ceil(((double)log(PGSIZE))/log(2));
    ptBits = (int)floor(((double)(32-offset))/2);
    pdBits = 32-(offset+ptBits);

    // Calculates the number of pages and maximum number of virtual pages
    phyPages = (int)floor(MEMSIZE / PGSIZE);
    if(phyPages > (int)pow(2, (32-offset))){
        maxPages = (int)pow(2, (32-offset)); 
    }else{
        maxPages = phyPages;
    }

    // Allocating physical memory and valid bit map
    while(!phy_mem)
        phy_mem = (void*)malloc(MEMSIZE);
    
    while(!valid_phy_mem)
        valid_phy_mem = (void*)malloc(maxPages/8);

    // Setting valid bits to 0
    reservedBits = 32*pow(2, pdBits);
    int i = 0;
    for(; i < reservedBits; i++){
        setNotValid(phy_mem, i);
    }
    memset(valid_phy_mem, 0, maxPages/8);

    // Reserve the pages for the reserve bits
    int numReservedPages = (int)ceil((double)4*pow(2, pdBits) / (double)PGSIZE);
    for(i = 0; i < numReservedPages; i++){
        setValid(valid_phy_mem, i);
    }

    // Sets valid variable for TLB
    for(int i = 0; i < TLB_ENTRIES; i++){
        tlb_store.entries[i].valid = 0;
    }
    
    maxVA = 0;
    pte_t* ptAdd = malloc(sizeof(pte_t));
    memset(ptAdd, 0, sizeof(pte_t));
    *ptAdd = 1;
    *ptAdd = *ptAdd << offset;
    for(int i = 0; i < maxPages-1; i++){
        maxVA += *ptAdd;
    }
    free(ptAdd);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex1, NULL);

    //debugPDE();
    //printf("%d, %d, %d, %d\n", pdBits, ptBits, offset, maxPages);
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 */
int
add_TLB(void *va, void *pa){

    // Gets rid of offset bits from va and pa
    pte_t vaddress = *(pte_t*)va;
    vaddress = vaddress >> offset;
    pte_t paddress = *(pte_t*)pa;
    paddress = paddress >> offset;

    // Add it to the TLB
    int index = vaddress % TLB_ENTRIES;
    tlb_store.entries[index].va = vaddress;
    tlb_store.entries[index].pa = paddress;
    tlb_store.entries[index].valid = 1;
    
    return 1;
}


/*
 * Check TLB for a valid translation.
 * Returns the physical page address otherwise return NULL
 */
pte_t
check_TLB(void *va) {

    // Gets the physical page index
    pte_t address = *(pte_t*)va;
    address = address >> offset;
    int index = address % TLB_ENTRIES;
    checks++;
    if(tlb_store.entries[index].va == address && tlb_store.entries[index].valid == 1){
        return tlb_store.entries[index].pa;
    }

    misses++;
    return -1;
}


/*
 * Prints TLB miss rate and percentage (to avoid confusion).
 */
void
print_TLB_missrate(){
    double miss_rate = (double)misses / (double)checks;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/

    fprintf(stderr, "TLB miss rate %lf, Percent rate: %lf%% \n", miss_rate, miss_rate*100);
}



/*
 * The function takes a virtual address and page directories starting address and
 * performs translation to return the physical address
*/
pte_t *translate(void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */
    
    if(*(pte_t*)va > maxVA)
        return NULL;
    // Gets pdIndex, ptIndex, and offsets
    int pdIndex = get_top_bits(*(pte_t*)va, pdBits);
    int ptIndex = get_mid_bits(*(pte_t*)va, ptBits, offset);
    int offsetVal = get_mid_bits(*(pte_t*)va, offset, 0);
    
    // Checks TLB for va
    pte_t tlbPA = check_TLB(va);
    if(tlbPA != -1){
        pte_t* lookup = (pte_t*)malloc(sizeof(pte_t));
        memset(lookup, 0, sizeof(pte_t));
        *lookup = tlbPA;
        *lookup = (*lookup << offset) | offsetVal;
        return lookup;
    }

    void* ptr = ((pte_t*)phy_mem) + pdIndex; // Get PD entry
    int valid = get_top_bits(*(pte_t*)ptr, 1); // Get valid PDE bit
    if(valid != 0){
        // PDE has an entry
        int ptPhyIndex = get_mid_bits(*(pte_t*)ptr, (ptBits + pdBits), 0); // Get PT phy address(index)
        void* pageTable = (char*)phy_mem + ptPhyIndex*PGSIZE; // Get ptr to PT
        pageTable = ((pte_t*)pageTable) + ptIndex; // Get the PT entry
        valid = get_top_bits(*(pte_t*)pageTable, 1); // Get valid PTE bit
        if(valid != 0){
            // PTE has an entry 
            pte_t* phyadr = (pte_t*)malloc(sizeof(pte_t)); // To malloc to to not malloc that is the question! The answer is YES! free later!!!
            memset(phyadr, 0, sizeof(pte_t));
            *phyadr = get_mid_bits(*(pte_t*)pageTable, (ptBits + pdBits), 0);
            *phyadr = (*phyadr << offset) | offsetVal;

            add_TLB(va, phyadr);
            return phyadr; // Return physical address(index)
        }
    }
    // If translation not successful thus is not a valid virtual address
    return NULL; 
}


/*
 * The function takes a page directory address, virtual address, physical address
 * as an argument, and sets a page table entry. This function will walk the page
 * directory to see if there is an existing mapping for a virtual address. If the
 * virtual address is not present, then a new entry will be added
 * 
 * Assumes that the Physical address valid bits are already set in the physical valid bitmap
*/
int
page_map(void *va, void *pa) {
    if(*(pte_t*)va > maxVA)
        return -1;
    // Gets pdIndex, ptIndex, and Offset (doesn't need a offset since it's just mapping va's to pa's)
    int pdIndex = get_top_bits(*(pte_t*)va, pdBits);
    int ptIndex = get_mid_bits(*(pte_t*)va, ptBits, offset);
    int offsetVal = get_mid_bits(*(pte_t*)va, offset, 0); 

    void* ptr = (pte_t*)phy_mem + pdIndex; // Get PDE
    int valid = get_top_bits(*(pte_t*)ptr, 1); // Check PDE's valid bit
    if(valid == 0){
        // No page table entry
        int validPhy = getValidPhyPage((int)ceil((double)4*pow(2, ptBits)/(double)PGSIZE)); // Check if there is enough physical memory space to store the PT and returns an index
        if(validPhy != -1) {
            // Sets valid bit and index bits for new PDE
            pte_t* pte = ptr;
            *pte = validPhy;
            setValid((char*)pte, 31);
            ptr = (char*)phy_mem + PGSIZE*validPhy; // Get base pointer to new PT
            
            memset(ptr, 0, PGSIZE*(int)ceil((double)4*pow(2, ptBits)/(double)PGSIZE)); // clears physical page(s)
            for(int i = validPhy; i < ((int)ceil((double)4*pow(2, ptBits)/(double)PGSIZE)+validPhy); i++){ // Sets the valid bits in physical bitmap
                setValid(valid_phy_mem, i);
            }
            ptr = (pte_t*)ptr + ptIndex;
            valid = get_top_bits(*(pte_t*)ptr, 1);
            if(valid == 0){
                // Virtual page has not been taken (will always happen)
                pte_t* phy_index = pa;
                int indexNum = get_mid_bits(*(pte_t*)phy_index, (ptBits + pdBits), 0); // Get phy index
                
                setValid(pa, 31); // Sets valid PTE bit    
                memcpy((pte_t*)ptr, phy_index, 4); // Maps phy bits to PTE 
                //debugPDE();
                // Mapping successful
                return 1;
                
            }else{
                // Virtual page has already been taken (impossible)
                //debugPDE(); 
                return -1;
            }

        }
        // Not enough physical pages to allocate new pagetable
    }else{
        // There exists a page table entry
        ptr = (char*)phy_mem + PGSIZE*get_mid_bits(*(pte_t*)ptr, (ptBits + pdBits), 0); // Base address of PT
        ptr = (pte_t*)ptr + ptIndex; // Get PTE
        valid = get_top_bits(*(pte_t*)ptr, 1); // Get Valid bit
        if(valid == 0){
            // Virtual page has not been taken
            pte_t* phy_index = pa;
            int indexNum = get_mid_bits(*(pte_t*)phy_index, (ptBits + pdBits), 0); // Get phy index
            
            setValid(pa, 31); // Sets valid PTE bit
            memcpy(ptr, phy_index, 4); // Maps phy bits to PTE
            //debugPDE(); // *************** delete
            // Mapping successful
            return 1;
            
        }else{
            // Virtual page has already been taken
            //debugPDE(); // *************** delete
            return -1;
        }
    }
    //debugPDE();
    return -1;
}

/*
 * Gets the starting index of num_pages valid continguous physical pages
*/
int getValidPhyPage(int num_pages) {
    for(int i = 0; i < maxPages; i++){
        if(getBit((char*)valid_phy_mem, i) == 0){
            int pages = 0;
            for(int j = i; j < maxPages; j++){
                if(getBit((char*)valid_phy_mem, j) == 1){
                    break;
                }
                pages++;
            }
            if(pages >= num_pages){
                return i;
            }
        }
    }
    return -1;
}

/*
 * Sets the next num_pages physical pages from index
*/
void setValidPhyPage(int index, int num_pages) {
    for(int i = index; i < (index+num_pages); i++) {
        setValid(valid_phy_mem, i);
    }
}

/*
 * Simple method to count the number of available physical pages in physical memory
*/
int numValidPhyPage(){
    int count = 0;
    for(int i = 0; i < maxPages; i++){
        if(getBit((char*)valid_phy_mem, i) == 0)
            count++;
    }
    return count;
}

/*
 * Adds one to PT bits to increment to the next virtual page
*/
pte_t* addPTBits(pte_t* va){
    pte_t* one = malloc(sizeof(pte_t));
    memset(one, 0, sizeof(pte_t));
    *one = 1;
    *one = *one << (32 - ptBits- pdBits);
    *va = *va + *one;
    free(one);
    return va;
}

/*
 * Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
    //Use virtual address bitmap to find the next free page
    pte_t* nextPage = malloc(sizeof(pte_t));
    memset(nextPage, 0, sizeof(pte_t));
    for(int i = 0; i < pow(2, (pdBits+ptBits)); i++){
        if(i != 0){
            nextPage = addPTBits(nextPage);
        }
        pte_t* curr = malloc(sizeof(pte_t));
        memset(curr, 0, sizeof(pte_t));
        *curr = *nextPage;
        int count = 0, countPTpage = 0;
        for(int j = i; j < pow(2, (pdBits+ptBits)); j++) {
            void* ptr = (pte_t*)phy_mem + get_top_bits(*curr, pdBits); // Get PDE
            if(get_top_bits(*(pte_t*)ptr, 1) == 0){ // only add count by one
                if(count < num_pages && (count+1) == num_pages){
                    PTPage = countPTpage;
                    PTFlag = 1;
                    return nextPage;
                }
                count++;
            }else{
                ptr = (char*)phy_mem + PGSIZE*get_mid_bits(*(pte_t*)ptr, (pdBits+ptBits), 0); // GET base PT
                if(get_top_bits(*((pte_t*)ptr + get_mid_bits(*curr, ptBits, offset)), 1) == 0){
                    
                    countPTpage++;
                    if(count < num_pages && (count+1) == num_pages){
                        PTPage = countPTpage;
                        PTFlag = 0;
                        return nextPage;
                    }
                    count++;
                }else{
                    break;
                }
            }

            curr = addPTBits(curr);
        }
        free(curr);
        
        if(count >= num_pages){
            // Probably impossible 
            return nextPage;
        }
        
    }
    return NULL;
}


/* 
 * Function responsible for allocating pages
*/
void *a_malloc(unsigned int num_bytes) {
    pthread_mutex_lock(&mutex);
    set_physical_mem();
    
    int numValidPhyPages = numValidPhyPage();

    if(num_bytes == 0 || numValidPhyPages == 0){ // Can't allocate 0 bytes and can't allocate pages if there are no free pages to allocate 
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    
    int numPage = (int)ceil((double)num_bytes / (double)PGSIZE);
    void* va = get_next_avail(numPage);
    if(va != NULL){
        if(PTFlag == 0){
            if((numValidPhyPages-numPage) < 0){
                // Not enough physical pages to allocate for this malloc
                //debugPDE();
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }else{
            if((ceil(ceil((numPage - PTPage) 
            / (double)pow(2, ptBits))*(4*pow(2, ptBits))
            /(double)PGSIZE) + numPage) > numValidPhyPages){
                // If the number of pages needed for a new PT + the number of pages needed to be allocated exceeds the number of free phy pages
                //debugPDE();
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
        mallocPages += numPage;
        
        pte_t* ptrVA = malloc(sizeof(pte_t));
        memset(ptrVA, 0, sizeof(pte_t));
        *ptrVA = *(pte_t*)va;
        for(int i = 0; i < numPage; i++){
            int phyIndex = getValidPhyPage(1);
            setValidPhyPage(phyIndex, 1);
            
            pte_t* pa = malloc(sizeof(pte_t));
            *pa = phyIndex;
            page_map(ptrVA, pa);
            ptrVA = addPTBits(ptrVA);
            free(pa);
        }
        free(ptrVA);
        
        //debugPDE(); 
        void* result = (void*)(*(pte_t*)va);
        pthread_mutex_unlock(&mutex);
        return result;
        
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

/*
 * Checks if the va is at the max possible va
*/
int isMaxVa(pte_t va) {
    for(int i = 31; i >= (offset); i--){
        int temp = (va >> i) & 1;
        if(temp == 0){
            return 0;
        } 
    }
    return 1;
}

/*
 * Checks if the PT is empty and is ready to be free'd
*/
int isEmpty(void* PT){
    void* ptr = PT;
    for(int i = 0; i < pow(2, ptBits); i++){
        if(get_top_bits(*(pte_t*)ptr, 1) == 1){
            return 0;
        }

        ptr = (pte_t*)PT + i;
    }
    return 1;
}

/* 
 * Responsible for releasing one or more memory pages using virtual address (va)
 * Will not free a va if it has an offset since it does not begin at 
 * the beginning of a page 
*/
void a_free(void *va, int size) {

    pthread_mutex_lock(&mutex);
    if(size <= 0 || (pte_t)va < 0){
        pthread_mutex_unlock(&mutex);
        return;
    }
    if((pte_t)va > maxVA){
        pthread_mutex_unlock(&mutex);
    
        return;
    }

    pte_t address = (pte_t)va;
    if(get_mid_bits(address, offset, 0) != 0){
        // Can't free address that don't begin at a page
        //debugPDE(); 
        //printf("Failed to free\n");
        pthread_mutex_unlock(&mutex);
        return;
    }
    int num_pages = (int)ceil((double)size / (double)PGSIZE);
    pte_t addPT = 1 << offset;
    pte_t checkAddress = address;

    for(int i = 0; i < num_pages; i++){
        if(i != 0)
            checkAddress += addPT; 
        
        if(isMaxVa((checkAddress)) == 1){
            if((i+1) < num_pages){
                // Specfied too many pages exceeds max va
                //debugPDE(); 
                pthread_mutex_unlock(&mutex);
                return;
            }
        }
        void* PD = (pte_t*)phy_mem + (checkAddress >> (offset+ptBits));
        if(get_top_bits(*(pte_t*)PD, 1) == 1) {
            void* PT = (char*)phy_mem + PGSIZE*get_mid_bits(*(pte_t*)PD, (ptBits+pdBits), 0);
            pte_t mask = (1 << ptBits) - 1;
            PT = (pte_t*)PT + ((checkAddress >> offset) & mask);
            if(get_top_bits(*(pte_t*)PT, 1) != 1){
                // PTE is already free'd
                //debugPDE(); 
                //printf("Failed to free\n");
                pthread_mutex_unlock(&mutex);
                return;
            }
        }else{
            // PDE is already free'd
            //debugPDE(); 
            pthread_mutex_unlock(&mutex);
            return;
        }     
    }

    pte_t ptr = address;
    for(int i = 0; i < num_pages; i++){
        if(i != 0)
            ptr += addPT;
        
        void* PD = (pte_t*)phy_mem + (ptr >> (offset+ptBits));
        void* PT = (char*)phy_mem + PGSIZE*get_mid_bits(*(pte_t*)PD, (ptBits+pdBits), 0);
        pte_t mask = (1 << ptBits) - 1;
        PT = (pte_t*)PT + ((ptr >> offset) & mask);

        int indexTLB = (ptr >> offset) % TLB_ENTRIES;
        if(tlb_store.entries[indexTLB].va == (ptr>>offset) && tlb_store.entries[indexTLB].valid == 1){
            tlb_store.entries[indexTLB].valid == 0;
        }

        setNotValid(valid_phy_mem, get_mid_bits(*(pte_t*)PT, (ptBits+pdBits), 0));
        setNotValid((char*)PT, 31);
        if(isEmpty(PT) == 1){
            setNotValid(valid_phy_mem, get_mid_bits(*(pte_t*)PD, (ptBits+pdBits), 0));
            setNotValid((char*)PD, 31);
        }
    }
    //debugPDE();
    mallocPages -= num_pages;
    /*
    if(mallocPages == 0){
        free(phy_mem);
        free(valid_phy_mem);
        phy_mem = NULL;
        valid_phy_mem = NULL;
    }
    */
    pthread_mutex_unlock(&mutex);
    return;
}


/* 
 * The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
*/
void put_value(void *va, void *val, int size) {

    pthread_mutex_lock(&mutex);
    

    pte_t address = (pte_t)va;
    
    int num_pages = (int)ceil((double)size / (double)PGSIZE);
    pte_t addPT = 1 << offset;
    pte_t checkAddress = address;
    for(int i = 0; i < num_pages; i++){
        if(i != 0)
            checkAddress += addPT;

        if(isMaxVa((checkAddress)) == 1){
            if((i+1) < num_pages){
                // Specfied too many pages exceeds max va
                //debugPDE(); // *************** delete
                pthread_mutex_unlock(&mutex);
                return;
            }
        }   

        void* PD = (pte_t*)phy_mem + (checkAddress >> (offset+ptBits));
        if(get_top_bits(*(pte_t*)PD, 1) == 1) {
            void* PT = (char*)phy_mem + PGSIZE*get_mid_bits(*(pte_t*)PD, (ptBits+pdBits), 0);
            pte_t mask = (1 << ptBits) - 1;
            PT = (pte_t*)PT + ((checkAddress >> offset) & mask);
            if(get_top_bits(*(pte_t*)PT, 1) != 1){
                // Trying to access a PTE that is not valid
                //debugPDE(); // *************** delete
                pthread_mutex_unlock(&mutex);
                return;
            }
        }else{
            // Trying to access a PDE that is not valid
            //debugPDE(); // *************** delete
            pthread_mutex_unlock(&mutex);
            return;
        }
    }

    int count = 0, currSize = size;
    pte_t* curr = malloc(sizeof(pte_t)); 
    *curr = address;
    for(int i = 0; i < num_pages; i++){
        if(i != 0)
            *curr += addPT;

        pte_t* phy = translate(curr);
        if(phy != NULL){
            void* ptr = (char*)phy_mem + PGSIZE*((*phy) >> offset); // Get phy page index
            ptr += ((*phy) & ((1 << offset) -1)); // Add offset
            
            for(int i = 0; i < (PGSIZE - ((*phy) & ((1 << offset) -1))); i++){
                ((char*)ptr)[i] = ((char*)val)[count];
                count++;
                currSize--;
                if(currSize == 0){
                    break;
                }
            }
            free(phy);
        }else{
            // Impossible
            free(curr);
            pthread_mutex_unlock(&mutex);
            return;
        }
        // Clear offset since it is the beginning of the next page 
        *curr = *curr >> offset;
        *curr = *curr << offset;
    }
    free(curr);
    //debugPDE(); // *************** delete
    pthread_mutex_unlock(&mutex);
    return;
}


/*
 * Given a virtual address, this function copies the contents of the page to val
 */
void get_value(void *va, void *val, int size) {

    pthread_mutex_lock(&mutex);


    pte_t address = (pte_t)va;
    
    int num_pages = (int)ceil((double)size / (double)PGSIZE);
    pte_t addPT = 1 << offset;
    pte_t checkAddress = address;
    for(int i = 0; i < num_pages; i++){
        if(i != 0)
            checkAddress += addPT;

        if(isMaxVa((checkAddress)) == 1){
            if((i+1) < num_pages){
                // Specfied too many pages exceeds max va
                //debugPDE(); // *************** delete
                pthread_mutex_unlock(&mutex);
                return;
            }
        }   

        void* PD = (pte_t*)phy_mem + (checkAddress >> (offset+ptBits));
        if(get_top_bits(*(pte_t*)PD, 1) == 1) {
            void* PT = (char*)phy_mem + PGSIZE*get_mid_bits(*(pte_t*)PD, (ptBits+pdBits), 0);
            pte_t mask = (1 << ptBits) - 1;
            PT = (pte_t*)PT + ((checkAddress >> offset) & mask);
            if(get_top_bits(*(pte_t*)PT, 1) != 1){
                // Trying to access a PTE that is not valid
                //debugPDE(); // *************** delete
                pthread_mutex_unlock(&mutex);
                return;
            }
        }else{
            // Trying to access a PDE that is not valid
            //debugPDE(); // *************** delete
            pthread_mutex_unlock(&mutex);
            return;
        }
    }

    int count = 0, currSize = size;
    pte_t* curr = malloc(sizeof(pte_t)); 
    *curr = address;
    for(int i = 0; i < num_pages; i++){
        if(i != 0)
            *curr += addPT;
        pte_t* phy = translate(curr);
        if(phy != NULL){
            void* ptr = (char*)phy_mem + PGSIZE*((*phy) >> offset); // Get phy page index
            ptr += ((*phy) & ((1 << offset) -1)); // Add offset
            for(int i = 0; i < (PGSIZE - ((*phy) & ((1 << offset) -1))); i++){
                ((char*)val)[count] = ((char*)ptr)[i];
                count++;
                currSize--;
                if(currSize == 0){
                    break;
                }
            }
        }else{
            // Impossible
            free(curr);
            pthread_mutex_unlock(&mutex);
            return;
        }
        // Clear offset since 
        *curr = *curr >> offset;
        *curr = *curr << offset;
    }
    free(curr);
    pthread_mutex_unlock(&mutex);
    return;

}



/*
 * This function receives two matrices mat1 and mat2 as an argument with size
 * argument representing the number of rows and columns. After performing matrix
 * multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {
    pthread_mutex_lock(&mutex1);
    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            int curr = 0;
            for(int k = 0; k < size; k++){
                int m1Ad = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int m2Ad = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                int m1Val, m2Val;
                get_value((void*)m1Ad, &m1Val, sizeof(int));
                get_value((void*)m2Ad, &m2Val, sizeof(int));
                curr += m1Val*m2Val;
            }
            int m3Ad = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            put_value((void*)m3Ad, &curr, sizeof(int));
        }
    }
    pthread_mutex_unlock(&mutex1);
       
}
