#include "elf.h"
#include "FS/vfs.h"
#include "Lib/debug.h"
#include "string.h"

namespace ELF
{
	PROCESS* Load(char* path)
	{
		char* image;
		int size = VFS::ReadFile(path, image);

		if (!size)
		{
			debug_print("File not found '%s'", path);
			return 0;
		}

		ELF32_HEADER* header = (ELF32_HEADER*)image;

		char sig[] = { 0x7F, 'E', 'L', 'F' };

		if (memcmp(&header->e_ident, &sig, sizeof(sig)))
		{
			//Not elf file
			debug_print("Invalid signature\n");
			return 0;
		}

		asm volatile("cli");

		ADDRESS_SPACE old_space = VMem::GetAddressSpace();
		ADDRESS_SPACE addr_space = VMem::CreateAddressSpace();
		VMem::SwapAddressSpace(addr_space);

		pflags_t flags = PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

		for (int i = 0; i < header->e_phnum; i++)
		{
			ELF32_PHEADER* pheader = (ELF32_PHEADER*)(image + header->e_phoff + i * header->e_phentsize);

			uint32 blocks = BYTES_TO_BLOCKS(pheader->p_memsz) + 1;

			void* phys = PMem::AllocBlocks(blocks);
			void* virt = (void*)pheader->p_vaddr;

			VMem::MapPages(virt, phys, blocks, flags);
			memcpy(virt, image + pheader->p_offset, pheader->p_memsz);
		}

		for (int i = 0; i < header->e_shnum; i++)
		{
			ELF32_SHEADER* sheader = (ELF32_SHEADER*)(image + header->e_shoff + i * header->e_shentsize);

			if (sheader->sh_type == ELF32_SHT_NOBITS)
			{
				memset((void*)sheader->sh_addr, 0, sheader->sh_size);
			}
		}

		debug_print("Loaded image %ux\n", addr_space.ptr);
		
		PROCESS* proc = new PROCESS(PROCESS_USER, addr_space);
		ProcessManager::CreateThread(proc, (void(*)())header->e_entry);

		VMem::SwapAddressSpace(old_space);

		asm volatile("sti");
		return proc;
	}
}