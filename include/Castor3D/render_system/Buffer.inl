//*************************************************************************************************

template <> struct BufferFunctor<char>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<char> > p_pBuffer)
	{
		return p_pManager->m_stCharBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<char> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stCharBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<char> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stCharBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<char> > p_pBuffer)
	{
		return p_pManager->m_stCharBuffers.DeleteBuffer( p_pBuffer);
	}
};

template <> struct BufferFunctor<unsigned char>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<unsigned char> > p_pBuffer)
	{
		return p_pManager->m_stUCharBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<unsigned char> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stUCharBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<unsigned char> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stUCharBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<unsigned char> > p_pBuffer)
	{
		return p_pManager->m_stUCharBuffers.DeleteBuffer( p_pBuffer);
	}
};

template <> struct BufferFunctor<bool>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<bool> > p_pBuffer)
	{
		return p_pManager->m_stBoolBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<bool> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stBoolBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<bool> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stBoolBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<bool> > p_pBuffer)
	{
		return p_pManager->m_stBoolBuffers.DeleteBuffer( p_pBuffer);
	}
};

template <> struct BufferFunctor<int>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<int> > p_pBuffer)
	{
		return p_pManager->m_stIntBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<int> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stIntBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<int> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stIntBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<int> > p_pBuffer)
	{
		return p_pManager->m_stIntBuffers.DeleteBuffer( p_pBuffer);
	}
};

template <> struct BufferFunctor<unsigned int>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<unsigned int> > p_pBuffer)
	{
		return p_pManager->m_stUIntBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<unsigned int> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stUIntBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<unsigned int> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stUIntBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<unsigned int> > p_pBuffer)
	{
		return p_pManager->m_stUIntBuffers.DeleteBuffer( p_pBuffer);
	}
};

template <> struct BufferFunctor<float>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<float> > p_pBuffer)
	{
		return p_pManager->m_stFloatBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<float> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stFloatBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<float> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stFloatBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<float> > p_pBuffer)
	{
		return p_pManager->m_stFloatBuffers.DeleteBuffer( p_pBuffer);
	}
};

template <> struct BufferFunctor<double>
{
	static bool Add( BufferManager * p_pManager, shared_ptr< Buffer3D<double> > p_pBuffer)
	{
		return p_pManager->m_stDoubleBuffers.AddBuffer( p_pBuffer);
	}
	static bool Assign( BufferManager * p_pManager, Buffer3D<double> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stDoubleBuffers.AssignBuffer( p_pBuffer, p_uiID);
	}
	static bool Unassign( BufferManager * p_pManager, Buffer3D<double> * p_pBuffer, size_t p_uiID)
	{
		return p_pManager->m_stDoubleBuffers.UnassignBuffer( p_pBuffer, p_uiID);
	}
	static bool Delete( BufferManager * p_pManager, shared_ptr< Buffer3D<double> > p_pBuffer)
	{
		return p_pManager->m_stDoubleBuffers.DeleteBuffer( p_pBuffer);
	}
};

//*************************************************************************************************

template <typename T>
void BufferManager::TBuffers<T> :: Cleanup()
{
	for (size_t i = 0 ; i < m_arrayBuffers.size() ; i++)
	{
		DeleteBuffer( m_arrayBuffers[i]);
	}

	m_arrayBuffers.clear();
	m_mapBuffersIndexes.clear();
	m_mapBuffers.clear();
}

template <typename T>
void BufferManager::TBuffers<T> :: Update()
{
	for (size_t i = 0 ; i < m_arrayBuffersToDelete.size() ; i++)
	{
		m_arrayBuffersToDelete[i]->Cleanup();
	}

	m_arrayBuffersToDelete.clear();
}

template <typename T>
bool BufferManager::TBuffers<T> :: AddBuffer( Buffer3DPtr p_pBuffer)
{
	bool l_bReturn = false;

	CASTOR_ASSERT( ! p_pBuffer == NULL);

	if (m_mapBuffersIndexes.find( p_pBuffer.get()) == m_mapBuffersIndexes.end())
	{
		m_arrayBuffers.push_back( p_pBuffer);
		l_bReturn = true;
	}

	return l_bReturn;
}

template <typename T>
bool BufferManager::TBuffers<T> :: AssignBuffer( Buffer3D<T> * p_pBuffer, size_t p_uiID)
{
	bool l_bReturn = false;
	CASTOR_ASSERT( p_pBuffer != NULL && p_uiID != 0);

	if (m_mapBuffers.find( p_uiID) == m_mapBuffers.end())
	{
		for (size_t i = 0 ; i < m_arrayBuffers.size() && ! l_bReturn ; i++)
		{
			if (m_arrayBuffers[i].get() == p_pBuffer)
			{
				m_mapBuffers.insert( typename Buffer3DPtrUIntMap::value_type( p_uiID, p_pBuffer));
				m_mapBuffersIndexes.insert( typename UIntBuffer3DPtrMap::value_type( p_pBuffer, p_uiID));
				l_bReturn = true;
			}
		}
	}

	return l_bReturn;
}

template <typename T>
bool BufferManager::TBuffers<T> :: UnassignBuffer( Buffer3D<T> * p_pBuffer, size_t p_uiID)
{
	bool l_bReturn = false;
	CASTOR_ASSERT( p_pBuffer != NULL && p_uiID != 0);
	typename Buffer3DPtrUIntMap::iterator l_it = m_mapBuffers.find( p_uiID);

	if (l_it != m_mapBuffers.end())
	{
		l_bReturn = true;

		if (m_mapBuffersIndexes.find( p_pBuffer) != m_mapBuffersIndexes.end())
		{
			m_mapBuffersIndexes.erase( m_mapBuffersIndexes.find( p_pBuffer));
		}
	}

	return l_bReturn;
}

template <typename T>
bool BufferManager::TBuffers<T> :: DeleteBuffer( Buffer3DPtr p_pBuffer)
{
	bool l_bReturn = false;
	CASTOR_ASSERT( ! p_pBuffer == NULL);
	typename UIntBuffer3DPtrMap::iterator l_it = m_mapBuffersIndexes.find( p_pBuffer.get());

	if (l_it != m_mapBuffersIndexes.end())
	{
		size_t l_uiID = l_it->second;
		vector::eraseValue( m_arrayBuffers, p_pBuffer);
		typename Buffer3DPtrUIntMap::iterator l_it2 = m_mapBuffers.find( l_it->second);

		if (l_it2 != m_mapBuffers.end())
		{
			if (p_pBuffer->IsAssigned())
			{
				UnassignBuffer( p_pBuffer.get(), l_uiID);
			}

			m_mapBuffers.erase( m_mapBuffers.find( l_uiID));
		}

		m_arrayBuffersToDelete.push_back( p_pBuffer);
	}

	return l_bReturn;
}

//*************************************************************************************************

template <typename T, typename _Ty>
shared_ptr<_Ty> BufferManager :: CreateBuffer()
{
	shared_ptr<_Ty> l_pBuffer( new _Ty);

	if ( ! BufferFunctor<T>::Add( GetSingletonPtr(), l_pBuffer))
	{
		l_pBuffer.reset();
	}

	return l_pBuffer;
}

template <typename _Ty>
shared_ptr<_Ty> BufferManager :: CreateVertexBuffer( const BufferElementDeclaration * p_pElements, size_t p_uiCount)
{
	shared_ptr<_Ty> l_pBuffer( new _Ty( p_pElements, p_uiCount));

	if ( ! BufferFunctor<real>::Add( GetSingletonPtr(), l_pBuffer))
	{
		l_pBuffer.reset();
	}

	return l_pBuffer;
}

template <typename T, typename _Ty>
shared_ptr<_Ty> BufferManager :: CreateBuffer( const String & p_strArg)
{
	shared_ptr<_Ty> l_pBuffer( new _Ty( p_strArg));

	if ( ! BufferFunctor<T>::Add( GetSingletonPtr(), l_pBuffer))
	{
		l_pBuffer.reset();
	}

	return l_pBuffer;
}

template <typename T>
bool BufferManager :: AssignBuffer( size_t p_uiID, Buffer3D<T> * p_pBuffer)
{
	return BufferFunctor<T>::Assign( GetSingletonPtr(), p_pBuffer, p_uiID);
}

template <typename T>
bool BufferManager :: UnassignBuffer( size_t p_uiID, Buffer3D<T> * p_pBuffer)
{
	return BufferFunctor<T>::Unassign( GetSingletonPtr(), p_pBuffer, p_uiID);
}

template <typename T>
bool BufferManager :: DeleteBuffer( shared_ptr< Buffer3D<T> > p_pBuffer)
{
	return BufferFunctor<T>::Delete( GetSingletonPtr(), p_pBuffer);
}

//*************************************************************************************************