//----------------------------------------------------------------------------
//
// ExceptionHelpers.h
//
// Auxiliary definitions used to handle exceptions.
//
// Copyright (c) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include <exception>
#include <comdef.h> 

#define  _COM_ERROR_EXCEPTION_HELPER_(_hr) \
	_com_error(_hr, nullptr, false)

#define CATCH_AND_RETURN_HRESULT    \
    catch(_com_error const &error)  \
    {  printf("2error.Error():%d %d %ws %ws\n",error.Error(),__LINE__,__FILE__);                             \
        return error.Error();       \
    }                               \
    catch(std::bad_alloc const &)   \
    {       printf("E_OUTOFMEMORY %d \n",__LINE__);                          \
        return E_OUTOFMEMORY;       \
    }                               \
    catch (...)                     \
    {          printf("E_FAIL  %d\n",__LINE__);                        \
        return E_FAIL;              \
    }

#define CATCH_AND_RETURN_BOOLEAN    \
    catch(std::bad_alloc const &)   \
    {                               \
        return false;               \
    }                               \
    catch (...)                     \
    {                               \
        return false;               \
    }

#define CATCH_AND_RETURN_DWORD      \
    catch(std::bad_alloc const &)   \
    {                               \
        return ERROR_NOT_ENOUGH_MEMORY;   \
    }                               \
    catch (...)                     \
    {                               \
        return ERROR_UNHANDLED_EXCEPTION; \
    }
