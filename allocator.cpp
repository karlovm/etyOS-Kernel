// Memory allocator constants and structure definitions
#define MEMORY_POOL_SIZE 32768  // 32 KB memory pool
#define MIN_BLOCK_SIZE 32       // Minimum block size (includes header)
#define BLOCK_ALLOCATED 1       // Flag for allocated blocks

// Memory block header structure
struct BlockHeader {
    unsigned int size;          // Size of the block including header
    unsigned int flags;         // Block flags (allocated/free)
    BlockHeader* next;          // Next block in the free list
    BlockHeader* prev;          // Previous block in the free list
};

// Global variables
static char memory_pool[MEMORY_POOL_SIZE] __attribute__((aligned(8)));
static BlockHeader* free_list = nullptr;
static bool allocator_initialized = false;

// Function prototypes
void init_memory_allocator();
void* kmalloc(unsigned int size);
void kfree(void* ptr);
BlockHeader* find_free_block(unsigned int size);
void split_block(BlockHeader* block, unsigned int size);
void merge_free_blocks();

// Initialize the memory allocator
void init_memory_allocator() {
    if (allocator_initialized) return;
    
    // Initialize the first block header
    BlockHeader* initial_block = (BlockHeader*)memory_pool;
    initial_block->size = MEMORY_POOL_SIZE;
    initial_block->flags = 0;  // Mark as free
    initial_block->next = nullptr;
    initial_block->prev = nullptr;
    
    // Set up the free list
    free_list = initial_block;
    allocator_initialized = true;
}

// Allocate memory
void* kmalloc(unsigned int size) {
    if (!allocator_initialized) init_memory_allocator();
    
    // Adjust size to include header and align to 8 bytes
    unsigned int aligned_size = (size + sizeof(BlockHeader) + 7) & ~7;
    if (aligned_size < MIN_BLOCK_SIZE) aligned_size = MIN_BLOCK_SIZE;
    
    // Find a suitable free block
    BlockHeader* block = find_free_block(aligned_size);
    if (!block) return nullptr;  // Out of memory
    
    // Split the block if it's too large
    if (block->size >= aligned_size + MIN_BLOCK_SIZE) {
        split_block(block, aligned_size);
    }
    
    // Remove block from free list
    if (block->prev) block->prev->next = block->next;
    if (block->next) block->next->prev = block->prev;
    if (block == free_list) free_list = block->next;
    
    // Mark block as allocated
    block->flags = BLOCK_ALLOCATED;
    
    // Return pointer to usable memory (after header)
    return (void*)((char*)block + sizeof(BlockHeader));
}

// Free memory
void kfree(void* ptr) {
    if (!ptr) return;
    
    // Get block header
    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    
    // Mark block as free
    block->flags = 0;
    
    // Add to free list
    block->next = free_list;
    block->prev = nullptr;
    if (free_list) free_list->prev = block;
    free_list = block;
    
    // Merge adjacent free blocks
    merge_free_blocks();
}

// Find a free block of sufficient size
BlockHeader* find_free_block(unsigned int size) {
    BlockHeader* current = free_list;
    while (current) {
        if (current->size >= size) return current;
        current = current->next;
    }
    return nullptr;
}

// Split a block into two parts
void split_block(BlockHeader* block, unsigned int size) {
    BlockHeader* new_block = (BlockHeader*)((char*)block + size);
    new_block->size = block->size - size;
    new_block->flags = 0;
    new_block->next = block->next;
    new_block->prev = block->prev;
    
    block->size = size;
    
    if (new_block->next) new_block->next->prev = new_block;
    if (new_block->prev) new_block->prev->next = new_block;
    else free_list = new_block;
}

// Merge adjacent free blocks
void merge_free_blocks() {
    BlockHeader* current = free_list;
    while (current && current->next) {
        if ((char*)current + current->size == (char*)current->next) {
            // Merge with next block
            current->size += current->next->size;
            current->next = current->next->next;
            if (current->next) current->next->prev = current;
        } else {
            current = current->next;
        }
    }
}