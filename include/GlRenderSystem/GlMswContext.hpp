/*
This source file is part of Castor3D (http://castor3d.developpez.com/castor3d.htm)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
the program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*/
#ifndef ___Gl_MSWContext___
#define ___Gl_MSWContext___

#ifdef _WIN32
namespace Castor3D
{
	class GlContext : public Context, public MemoryTraced<GlContext>
    {
    protected:
        HDC m_hDC;
        HGLRC m_hContext;
		HWND m_hWnd;

	public:
		GlContext();
        GlContext( RenderWindow * p_window);
        virtual ~GlContext();

		virtual bool Initialise() { return m_bInitialised; }
        virtual void SetCurrent();
        virtual void EndCurrent();
        virtual void SwapBuffers();

		inline HDC		GetHDC		()const { return m_hDC; }
		inline HGLRC	GetContext	()const { return m_hContext; }

	protected:
		virtual void _createGlContext();
		virtual void _createGl3Context();
    };
}
#endif

#endif