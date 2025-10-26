- ajouter support plusieurs textures
- c'est quoi un sémaphore ?
- A quoi servent les queues ?
- refactoring caméra
- setup facile du projet
- shadow mapping
- post-processing

# Bugs à corriger

- validation layer: vkQueueSubmit(): pSubmits[0].pWaitSemaphores[0] queue (VkQueue 0x261020c6800) is waiting on semaphore (VkSemaphore 0x7c000000007c) that has no way to be signaled.
The Vulkan spec states: All elements of the pWaitSemaphores member of all elements of pSubmits created with a VkSemaphoreType of VK_SEMAPHORE_TYPE_BINARY must reference a semaphore signal operation that has been submitted for execution and any semaphore signal operations on which it depends must have also been submitted for execution (https://vulkan.lunarg.com/doc/view/1.4.313.2/windows/antora/spec/latest/chapters/cmdbuffers.html#VUID-vkQueueSubmit-pWaitSemaphores-03238).

- résoudre problème callback non appelé dans ODWindow (ou mettre static ODWindow* g_currentWindow = nullptr ?)

- Pourquoi anti-aliasing limité à VK_SAMPLE_COUNT_8_BIT ?

- problème touches zqsd