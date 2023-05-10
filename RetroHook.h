#pragma once

#include <winnt.h>

namespace RetroHook_Util
{
    /// <summary>
    /// Return the address of the virtual function in a pointer at an index.
    /// </summary>
    /// <param name="classBase">The class address to find the virtual index at.</param>
    /// <param name="index">The index to find in the class.</param>
    /// <returns></returns>
    inline uintptr_t GetVirtualAddr(void* classBase, size_t index) 
    {
        return (uintptr_t)(*(uintptr_t**)classBase)[index];
    }
}

enum RetroCodes_t
{
    RC_INITIALIZED,        // Successfully initialized. (Constructor)
    RC_HOOK_SUCCESS,       // Hook function succeeded.
    RC_HOOK_FAIL,          // Hook function failed.
    RC_BAD_BASE,           // Base class base address.
    RC_BAD_INDEX,          // Invalid index.
    RC_BAD_FUNCTION,       // Invalid function address.
    RC_BAD_FUNCTION_COUNT, // Invalid function count.
};

class RetroHook
{
private:
    RetroCodes_t m_currentStatus{};
    uintptr_t**  m_vtableAddr     = nullptr;
    uintptr_t*   m_originalAddr   = nullptr;
    uintptr_t*   m_pVmt           = nullptr;
    size_t       m_functionCount  = 0;
    size_t       m_hookIndex      = 0;
    void*        m_functionToHook = nullptr;
    void*        m_classBase      = nullptr;
    bool         m_getHookState   = 0;

public:
    /// <summary>
    /// Get current state of RetroHook.
    /// </summary>
    /// <returns></returns>
    inline RetroCodes_t GetStatus()
    {
        return m_currentStatus;
    }

    /// <summary>
    /// Determine if the function is hooked.
    /// </summary>
    /// <returns></returns>
    inline bool IsHooked()
    {
        return m_getHookState;
    }

    /// <summary>
    /// RetroHook initializer that sets up all of our data.
    /// </summary>
    /// <param name="classBase">The address of the class, which we will use to find the virtual function.</param>
    /// <param name="hookIndex">The index of the function in the class.</param>
    /// <param name="functionToHook">Our custom-defined function to redirect code-execution to.</param>
    inline RetroHook(void* classBase, size_t hookIndex, void* functionToHook)
    {
        // Store members.
        m_classBase      = classBase;
        m_hookIndex      = hookIndex;
        m_functionToHook = functionToHook;

        if (!m_classBase)
        {
            m_currentStatus = RC_BAD_BASE;
            return;
        }

        if (!m_functionToHook)
        {
            m_currentStatus = RC_BAD_FUNCTION;
            return;
        }

        m_vtableAddr = reinterpret_cast<uintptr_t**>(classBase);

        // Acquire the amount of functions inside the virtual class.
        while (reinterpret_cast<uintptr_t*>(*m_vtableAddr)[m_functionCount])
        {
            m_functionCount++;
        }

        if (m_functionCount <= 0)
        {
            m_currentStatus = RC_BAD_FUNCTION_COUNT;
            return;
        }

        if (m_hookIndex < 0 || hookIndex > m_functionCount)
        {
            m_currentStatus = RC_BAD_INDEX;
            return;
        }

        // Set the original.
        m_originalAddr = *m_vtableAddr;

        m_pVmt = new uintptr_t[m_functionCount + 1];

        // Make a copy of the entire table.
        memcpy(m_pVmt, &m_originalAddr[-1], (sizeof(uintptr_t) * m_functionCount) + sizeof(uintptr_t));

        m_currentStatus = RC_INITIALIZED;
    }

    /// <summary>
    /// Apply the virtual hook in the class at the index replacing it with our function.
    /// </summary>
    /// <returns></returns>
    inline bool SetHook()
    {
        bool isValidHook = m_functionToHook && m_hookIndex >= 0 && m_hookIndex <= m_functionCount;
        if (isValidHook)
        {
            m_pVmt[m_hookIndex + 1] = reinterpret_cast<uintptr_t>(m_functionToHook);
            *m_vtableAddr = &m_pVmt[1];

            m_getHookState  = true;
            m_currentStatus = RC_HOOK_SUCCESS;

            return true;
        }

        m_currentStatus = RC_HOOK_FAIL;
        return false;
    }

    /// <summary>
    /// Restores the address change from our replacement back to the original.
    /// </summary>
    /// <returns></returns>
    inline bool RemoveHook()
    {
        // Ensure that we don't remove a hook that doesn't need to be removed.
        if (*m_vtableAddr != m_originalAddr)
        {
            *m_vtableAddr = m_originalAddr;
            delete m_pVmt;

            m_getHookState  = false;
            m_currentStatus = RC_HOOK_SUCCESS;

            return true;
        }

        m_currentStatus = RC_HOOK_FAIL;
        return false;
    }

    /// <summary>
    /// Acquires the address of the virtual function in the class.
    /// </summary>
    /// <returns></returns>
    inline uintptr_t* GetOriginalAddr(bool useVirtualFunction)
    {
        if (!m_classBase || m_hookIndex < 0 || m_hookIndex > m_functionCount)
            return 0;

        return useVirtualFunction ? (uintptr_t*)RetroHook_Util::GetVirtualAddr(m_classBase, m_hookIndex) : m_originalAddr;
    }

    /// <summary>
    /// Aquires the address after the hook has been applied.
    /// </summary>
    /// <returns></returns>
    inline uintptr_t* GetHookedAddr()
    {
        return IsHooked() ? m_pVmt : m_originalAddr;
    }
};
