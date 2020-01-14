//---------------------------------------------------------------------------
/*  Created 7-Sep-2003: Thomas Mulgrew

    Variable storage formats and data structures.                 
*/

#ifndef vmDataH
#define vmDataH
#include "vmTypes.h"
#include <list>
#include <set>
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// vmValue
//
// Used to store a single value.
// Used internally by registers, stack entries and variables.

#pragma pack (push, 1)
struct vmValue {
    union {
        vmInt           m_intVal;
        vmReal          m_realVal;
    } m_val;
    // Note:    When storing a string, the actual value is stored in a separate
    //          string array. m_intVal then stores the index of the string in
    //          this array.

    vmInt   &IntVal ()  { return m_val.m_intVal; }
    vmReal  &RealVal () { return m_val.m_realVal; }
	vmInt	IntVal() const { return m_val.m_intVal; }
	vmReal	RealVal() const { return m_val.m_realVal; }

    vmValue () {                        // Default constructor
        m_val.m_intVal = 0;
    }
    vmValue (const vmValue& v) {        // Copy constructor
        m_val = v.m_val;
    }
    vmValue (vmInt intVal) {
        m_val.m_intVal = intVal;
    }
    vmValue (vmReal realVal) {
        m_val.m_realVal = realVal;
    }

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};
#pragma pack (pop)

////////////////////////////////////////////////////////////////////////////////
// vmStore
//
// Abstract template class for allocating and referencing a specific type of
// object.
// Used for strings, pointers, handles e.t.c.
// The virtual machine stores only the array index, and thus VM programs avoid
// having to see and manipulate pointers, handles e.t.c.
template<class T> class vmStore {
    std::vector<T>      m_array;
    std::vector<bool>   m_valAllocated;
    std::list<int>      m_freeList;             // List of free indices
    T                   m_blankElement;         // New elements are initialised to this

public:
    vmStore (T blankElement) : m_blankElement (blankElement) { ; }
    bool IndexValid (int index) {               // Return true if index is a valid allocated index
        return      index >= 0
                &&  (unsigned int)index < m_array.size ()
                &&  m_valAllocated [index];
    }
    bool IndexStored (int index) {
        return index != 0 && IndexValid (index);
    }
    T& Value (int index) {
        assert (IndexValid (index));
        return m_array [index];
    }
    int Alloc ();
    void Free (int index);
    void Clear ();
    int StoredElements() { return m_array.size() - m_freeList.size(); }

    std::vector<T>&     Array ()        { return m_array; }
    std::vector<bool>&  ValAllocated () { return m_valAllocated; }
    T&                  BlankElement () { return m_blankElement; }
	int MaxIndex() { return m_array.size() - 1; }
};

template<class T> int vmStore<T>::Alloc () {
    int index;
    if (m_freeList.empty ()) {

        // Extend array by a single item, and return index of that item
        index = m_array.size ();
        m_array.push_back (m_blankElement);
        m_valAllocated.push_back (true);        // Mark element as in use
    }
    else {

        // Reuse previously freed index
        std::list<int>::iterator i = m_freeList.begin ();
        index = *i;
        m_freeList.erase (i);

        // Initialise element
        m_array [index]         = m_blankElement;
        m_valAllocated [index]  = true;
    }
    return index;
}

template<class T> void vmStore<T>::Free (int index) {

    // Deallocate index and return to array
    assert (IndexValid (index));
    m_valAllocated [index]  = false;
    m_freeList.insert (m_freeList.begin (), index);
}

template<class T> void vmStore<T>::Clear () {

    // Clear allocated values
    m_freeList.clear ();
    m_array.clear ();
    m_valAllocated.clear ();

    // Allocate a "blank" value for the 0th element.
    // Basic4GL uses 0 to indicate that data hasn't been allocated yet.
    Alloc ();
}

////////////////////////////////////////////////////////////////////////////////
// vmValueStack
//
// Used to stack values for reverse-Polish expression evaluation, or as
// function parameters.
class vmValueStack {
    std::vector<vmValue>    m_data;
    vmStore<vmString>&      m_strings;

public:
    vmValueStack (vmStore<vmString>& strings) : m_strings (strings) {
        m_data.clear ();
    }

    bool Empty () { return m_data.empty (); }
    void Push (vmValue& v) {                        // Push v as NON string
        m_data.push_back (v);
    }
    void PushString (std::string& str) {
        int index = m_strings.Alloc ();             // Allocate string
        m_strings.Value (index) = str;              // Copy value
        m_data.push_back (vmValue (index));         // Create stack index
    }
    vmValue& TOS () {
        assert (!Empty ());
        return m_data [m_data.size () - 1];
    }
    void Pop (vmValue& v) {
        v = TOS ();
        m_data.pop_back ();
    }
    void PopString (std::string& str) {
        assert (!Empty ());

        // Copy string value from stack
        int index = TOS ().IntVal ();
        assert (m_strings.IndexValid (index));
        str = m_strings.Value (index);

        // Deallocate stacked string
        m_strings.Free (index);

        // Remove stack element
        m_data.pop_back ();
    }
    void Clear ()                   { m_data.clear (); }
    int Size ()                     { return m_data.size (); }
    vmValue& operator[] (int index) { return m_data [index]; }
    void Resize (int size)          { m_data.resize (size); }
};

////////////////////////////////////////////////////////////////////////////////
// vmResources
//
// Abstract base class for objects that store resources.

class vmResources {
public:
	virtual ~vmResources() {}
	virtual void Clear () = 0;          // Clear all resources
};

////////////////////////////////////////////////////////////////////////////////
// vmIntHandleResources
//
// For resource types that are represented as an integer handle.
// There is no real advantage of creating a further integer->resource mapping
// as the resource is already an integer. Therefore we simply record which
// handles have been allocated so that we can deallocate them.

class vmIntHandleResources : public vmResources {
protected:
    std::set<int> m_handles;
    virtual void DeleteHandle (int handle) = 0;
public:
    virtual void Clear ();
    bool Valid (int handle) { return m_handles.find (handle) != m_handles.end (); }
    void Store (int handle) { m_handles.insert (handle); }
    void Remove (int handle) {
        std::set<int>::iterator i = m_handles.find (handle);
        if (i != m_handles.end ())
            m_handles.erase (i);
    }
    void Free (int handle) {
        DeleteHandle (handle);
        Remove (handle);
    }
};

////////////////////////////////////////////////////////////////////////////////
// vmResourceStore
//
// Template implementation of vmResources, using an internal vmStore

template<class T> class vmResourceStore : public vmResources {
protected:
    vmStore<T> m_store;
    virtual void DeleteElement (int index) = 0;
public:
    vmResourceStore (T blankElement) : m_store (blankElement) { ; }

    virtual void Clear ();

    // Pass calls through to internal store
    bool IndexValid (int index)     { return m_store.IndexValid (index); }
    bool IndexStored (int index)    { return m_store.IndexStored (index); }
    T& Value (int index)            { return m_store.Value (index); }
    int Alloc ()                    { return m_store.Alloc (); }
    int Alloc (const T& element) {
        if (element == m_store.BlankElement ())
            return -1;
        else {
            int index = Alloc ();
            Value (index) = element;
            return index;
        }
    }
    void Remove (int index) {               // Remove an element without freeing it. Calling code is responsible for freeing the resource.
        m_store.Free (index);
    }
    void Free (int index) {                 // Remove and free element.
        if (IndexValid (index)) {
            DeleteElement (index);
            Remove (index);
        }
    }

    vmStore<T>& Store() { return m_store; }
};

template<class T> void vmResourceStore<T>::Clear () {

    // Delete each individual element
    for (unsigned i = 0; i < m_store.Array ().size (); i++)
        if (m_store.ValAllocated () [i])
            DeleteElement (i);

    // Clear store
    m_store.Clear ();
}

////////////////////////////////////////////////////////////////////////////////
// vmPointerResourceStore
//
// A vmResourceStore of pointers.
// NULL = blank.
// Pointer is deleted when removed.

template<class T> class vmPointerResourceStore : public vmResourceStore<T *> {
protected:
    virtual void DeleteElement (int index);
public:
    vmPointerResourceStore () : vmResourceStore<T *> (NULL) { ; }
};

template<class T> void vmPointerResourceStore<T>::DeleteElement (int index) {
    delete vmResourceStore<T*>::Value (index);                   // delete pointer
}

////////////////////////////////////////////////////////////////////////////////
// vmData
//
// A resizeable array of vmValue objects
typedef std::vector<vmValue> vmValueArray;
class vmData {

    // Data layout (m_data array)
    // -------------------------------------------------------------------------
    // 0                    Reserved for null pointers
    // -------------------------------------------------------------------------
    // 1                    Temporary data (allocated during expression
    // :                    evaluation. Grows downward)
    // m_tempData - 1
    // -------------------------------------------------------------------------
    // m_tempData
    // :                    Unused stack/temp space
    // m_stackTop - 1
    // -------------------------------------------------------------------------
    // m_stackTop           Local variable/parameter stack space. Used by user
    // :                    defined functions.
    // m_permanent - 1
    // -------------------------------------------------------------------------
    // m_permanent
    // :                    Permanent (global) variable storage
    // size()-1
    // -------------------------------------------------------------------------
    //
    // Stack overflow occurs when m_tempData > m_stackTop.
    // Out of memory occurs when size() > m_maxDataSize.
    vmValueArray    m_data;
    int             m_tempData;
    int             m_stackTop;
    int             m_permanent;
    int             m_maxDataSize;
        // Maximum # of permanent data values that can be stored.
        // Note: Caller must be sure to call RoomFor before calling Allocate to
        // ensure there is room for the data.

    int m_tempDataLock;             // Temp data below this point will NOT be
                                    // freed when FreeTempData is called.

    int InternalAllocate (int count) {

        // Allocate "count" elements and return iterator pointing to first one
        int top = Size ();
        if (count > 0)
            m_data.resize (Size () + count, vmValue ());
        return top;
    }

public:
    vmData (int maxDataSize, int stackSize) {
        assert(stackSize > 1);
        assert(maxDataSize > stackSize);

        // Ensure the maxDataSize is less than the maximum # elements supported
        // by the vector.
        if ((unsigned)maxDataSize > m_data.max_size())
            maxDataSize = m_data.max_size();
        assert(maxDataSize > stackSize);

        // Initialize data
        m_maxDataSize = maxDataSize;
        m_permanent = stackSize;
        Clear ();
    }
    vmValueArray& Data ()       { return m_data; }
    int MaxDataSize ()          { return m_maxDataSize; }
    int Permanent()             { return m_permanent; }
    int StackTop()              { return m_stackTop; }
    int TempData()              { return m_tempData; }
    int TempDataLock()          { return m_tempDataLock; }

    void Clear ()               {
        // Clear existing data
        m_data.clear();

        // Allocate stack
        m_data.resize(m_permanent, vmValue());

        // Clear temp data
        m_tempData = 1;
        m_tempDataLock = 1;

        // Clear stack
        m_stackTop = m_permanent;
    }
    int Size ()                 { return m_data.size (); }
    bool IndexValid (int i)     { return i >= 0 && i < Size (); }
    void InitData (int i, vmValType& type, vmTypeLibrary& typeLib);             // Initialise a new block of data

    // Permanent data
    int Allocate (int count) {
        assert(count >= 0);
        assert(RoomFor(count));

        // Allocate "count" elements and return iterator pointing to first one
        int top = m_data.size();
        if (count > 0)
            m_data.resize(m_data.size() + count, vmValue());
        return top;
    }
    bool RoomFor (int count) {
        assert(count >= 0);
        return m_maxDataSize - m_data.size() >= (unsigned)count;
    }

    // Stack data
    int AllocateStack(int count) {
        assert(count >= 0);
        assert(StackRoomFor(count));

        // Allocate stack data (stack grows downward)
        m_stackTop -= count;

        // Initialize data
        for (int i = 0; i < count; i++)
            m_data[m_stackTop + i] = vmValue();

        // Return index of start of data
        return m_stackTop;
    }
    bool StackRoomFor(int count) {
        assert(count >= 0);
        return m_stackTop - m_tempData >= count;
    }

    // Temporary data
    int AllocateTemp(int count, bool initData) {
        assert(count >= 0);
        assert(StackRoomFor(count));

        // Mark temp data position
        int top = m_tempData;

        // Allocate data
        m_tempData += count;

        // Initialize data
        if (initData)
            for (int i = 0; i < count; i++)
                m_data[top + i] = vmValue();

        // Return index of start of data
        return top;
    }

    // Lock the current temporary data so that it will not be freed by
    // FreeTempData(). Returns the previous lock point, which can be passed to
    // UnlockTempData().
    int LockTempData() {
        int prev = m_tempDataLock;
        m_tempDataLock = m_tempData;
        return prev;
    }

    // Unlock temporary data to a previous lock point (presumably returned by
    // LockTempData()
    void UnlockTempData(int newLockPosition) {
        assert(newLockPosition >= 1);
        assert(newLockPosition <= m_tempDataLock);
        m_tempDataLock = newLockPosition;
    }

    void FreeTemp() {
        m_tempData = m_tempDataLock;
    }

    void SaveState(
            int& stackTop,
            int& tempDataLock) {
        stackTop = m_stackTop;
        tempDataLock = LockTempData();
    }

    void RestoreState(
            int stackTop,
            int tempDataLock,
            bool freeTempData) {

        // Restore stack
        m_stackTop = stackTop;

        // Free temp data used after state was saved
        if (freeTempData)
            FreeTemp();

        // Unlock temp data from before save
        UnlockTempData(tempDataLock);
    }

/*    void GetState(
        unsigned int& size,
        unsigned int& stackTop,
        unsigned int& tempDataLock) {

        // Return state data
        size            = m_data.size ();
        stackTop        = m_stackTop;
        tempDataLock    = LockTempData();

        // Note: This is used by debugging. The state is captured and preserved
        // while temporary code is executed (e.g. evaluating a watch expression).
        // The state is then restored so that the program can continue executing.
        // We mark the temporary data as permanent, so it doesn't get thrown away
        // by the temporary code. (We then set it back to temporary once the
        // state is restored.)
    }
    void SetState (unsigned int size, unsigned int tempStart) {

        // Restore top of data
        if (size < m_data.size ())
            m_data.resize(size);

        // Restore stack
        m_stackTop = stackTop;

        // Free temp data used by

    }*/
};

////////////////////////////////////////////////////////////////////////////////
// vmProgramDataElement
//
// General purpose program data (as allocated with "DATA" statement in BASIC).
class vmProgramDataElement {
    vmBasicValType  m_type;
    vmValue         m_value;
public:
    vmBasicValType& Type ()     { return m_type; }
    vmValue&        Value ()    { return m_value; }

    // Streaming
#ifdef VM_STATE_STREAMING
    void StreamOut (std::ostream& stream) const;
    void StreamIn (std::istream& stream);
#endif
};

typedef std::vector<vmProgramDataElement> vmProgramData;

////////////////////////////////////////////////////////////////////////////////
// Misc functions

// Converting arrays to/from C style arrays
template<class T> int ReadArray (   vmData& data,           // Data
                                    int index,              // Index of object in data
                                    vmValType type,         // Data type
                                    T *array,               // Destination array
                                    int maxSize) {          // Maximum # of elements
    assert (type.m_basicType == VTP_INT || type.m_basicType == VTP_REAL);
    assert (type.VirtualPointerLevel () == 0);
    assert (type.m_arrayLevel > 0);
    assert (data.IndexValid (index));
    assert (array != NULL);
    assert (maxSize > 0);

    if (type.m_byRef)
        type.m_pointerLevel--;
    type.m_byRef = false;

    // Convert Basic4GL format array to C format array
    vmValType elementType = type;
    elementType.m_arrayLevel--;

    int elementCount = data.Data () [index].IntVal ();
    int elementSize  = data.Data () [index + 1].IntVal ();
    if (elementType.m_arrayLevel > 0) {
        int arrayOffset = 0;
        for (int i = 0; i < elementCount && arrayOffset < maxSize; i++)
            arrayOffset += ReadArray (  data,
                                        index + 2 + i * elementSize,
                                        elementType,
                                        &array [arrayOffset],
                                        maxSize - arrayOffset);
        return arrayOffset;
    }
    else if (elementType == VTP_INT) {
        if (elementCount > maxSize)
            elementCount = maxSize;
        for (int i = 0; i < elementCount; i++)
            array [i] = (T) data.Data () [index + 2 + i].IntVal ();
        return elementCount;
    }
    else if (elementType == VTP_REAL) {
        if (elementCount > maxSize)
            elementCount = maxSize;
        for (int i = 0; i < elementCount; i++)
            array [i] = (T) data.Data () [index + 2 + i].RealVal ();
        return elementCount;
    }
    else
        assert (false);
    return 0;
}

template<class T> int WriteArray (  vmData& data,           // Data
                                    int index,              // Index of object in data
                                    vmValType type,         // Data type
                                    T *array,               // Destination array
                                    int maxSize) {          // Maximum # of elements
    assert (type.m_basicType == VTP_INT || type.m_basicType == VTP_REAL);
    assert (type.VirtualPointerLevel () == 0);
    assert (type.m_arrayLevel > 0);
    assert (data.IndexValid (index));
    assert (array != NULL);
    assert (maxSize > 0);

    if (type.m_byRef)
        type.m_pointerLevel--;
    type.m_byRef = false;

    // Convert Basic4GL format array to C format array
    vmValType elementType = type;
    elementType.m_arrayLevel--;

    int elementCount = data.Data () [index].IntVal ();
    int elementSize  = data.Data () [index + 1].IntVal ();
    if (elementType.m_arrayLevel > 0) {
        int arrayOffset = 0;
        for (int i = 0; i < elementCount && arrayOffset < maxSize; i++)
            arrayOffset += WriteArray ( data,
                                        index + 2 + i * elementSize,
                                        elementType,
                                        &array [arrayOffset],
                                        maxSize - arrayOffset);
        return arrayOffset;
    }
    else if (elementType == VTP_INT) {
        if (elementCount > maxSize)
            elementCount = maxSize;
        for (int i = 0; i < elementCount; i++)
            data.Data () [index + 2 + i].IntVal () = array [i];
        return elementCount;
    }
    else if (elementType == VTP_REAL) {
        if (elementCount > maxSize)
            elementCount = maxSize;
        for (int i = 0; i < elementCount; i++)
            data.Data () [index + 2 + i].RealVal () = array [i];
        return elementCount;
    }
    else
        assert (false);
    return 0;
}

template<class T> int ReadAndZero ( vmData& data,           // Data
                                    int index,              // Index of object in data
                                    vmValType type,         // Data type
                                    T *array,               // Destination array
                                    int maxSize) {          // Maximum # of elements

    // Note: This template function only works for numeric types
    // Read array
    int size = ReadArray (data, index, type, array, maxSize);

    // Zero remaining elements
    for (int i = size; i < maxSize; i++)
        array [i] = (T) 0;

    return size;
}

template<class T> void ZeroArray (T *array, int size) {
    for (int i = 0; i < size; i++)
        array [i] = 0;
}

inline int TempArray (vmData& data, vmTypeLibrary& typeLib, vmBasicValType elementType, int arraySize) {
    assert (arraySize > 0);

    // Setup a basic 1D array of vmInts
    vmValType type = elementType;
    type << arraySize;                                          // Set array size

    // Allocate temporary array
    int dataIndex = data.AllocateTemp(typeLib.DataSize(type), true);
    data.InitData (dataIndex, type, typeLib);
    return dataIndex;
}

template<class T> int FillTempIntArray (vmData& data, vmTypeLibrary& typeLib, int arraySize, T *array) {
    assert (array != NULL);

    // Allocate temporary array
    int dataIndex = TempArray (data, typeLib, VTP_INT, arraySize);

    // Translate C array into data
    for (int i = 0; i < arraySize; i++)
        data.Data () [dataIndex + 2 + i].IntVal () = array [i];

    // Return temporary index
    return dataIndex;
}

template<class T> int FillTempRealArray (vmData& data, vmTypeLibrary& typeLib, int arraySize, T *array) {
    assert (array != NULL);

    // Allocate temporary array
    int dataIndex = TempArray (data, typeLib, VTP_REAL, arraySize);

    // Translate C array into data
    for (int i = 0; i < arraySize; i++)
        data.Data () [dataIndex + 2 + i].RealVal () = array [i];

    // Return temporary index
    return dataIndex;
}

inline int TempArray2D (vmData& data, vmTypeLibrary& typeLib, vmBasicValType elementType, int arraySize1, int arraySize2) {
    assert (arraySize1 > 0);
    assert (arraySize2 > 0);

    // Setup a basic 1D array of vmInts
    vmValType type = elementType;
    type << arraySize1 << arraySize2;               // Set array size

    // Allocate temporary array
    int dataIndex = data.AllocateTemp(typeLib.DataSize(type), true);
    data.InitData (dataIndex, type, typeLib);
    return dataIndex;
}

template<class T> int FillTempIntArray2D (vmData& data, vmTypeLibrary& typeLib, int arraySize1, int arraySize2, T *array) {
    assert (array != NULL);

    // Allocate temporary array
    int dataIndex = TempArray2D (data, typeLib, VTP_INT, arraySize1, arraySize2);

    // Translate C array into data
    int i = 0;
    for (int x = 0; x < arraySize1; x++) {
        int offset = dataIndex + x * data.Data () [dataIndex + 1].IntVal () + 2;
        for (int y = 0; y < arraySize2; y++)
            data.Data () [offset + y + 2].IntVal () = array [i];
    }

    // Return temporary index
    return dataIndex;
}

template<class T> int FillTempRealArray2D (vmData& data, vmTypeLibrary& typeLib, int arraySize1, int arraySize2, T *array) {
    assert (array != NULL);

    // Allocate temporary array
    int dataIndex = TempArray2D (data, typeLib, VTP_REAL, arraySize1, arraySize2);

    // Translate C array into data
    int i = 0;
    for (int x = 0; x < arraySize1; x++) {
        int offset = dataIndex + x * data.Data () [dataIndex + 1].IntVal () + 2;
        for (int y = 0; y < arraySize2; y++)
            data.Data () [offset + y + 2].RealVal () = array [i++];
    }

    // Return temporary index
    return dataIndex;
}

inline int ArrayDimensionSize (vmData& data, int arrayOffset, int dimension) {
    assert (data.IndexValid (arrayOffset));
    int index = arrayOffset + dimension * 2;
    assert (data.IndexValid (index));
    return data.Data () [index].IntVal ();
}

#endif
