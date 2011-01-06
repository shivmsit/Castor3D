#ifndef ___CS_NEWICOSAEDRONDIALOG___
#define ___CS_NEWICOSAEDRONDIALOG___

#include "Module_Geometry.h"
#include "NewGeometryDialog.h"

namespace CastorShape
{
	class NewIcosaedronDialog : public NewGeometryDialog
	{
	public:
		NewIcosaedronDialog( Castor3D::MaterialManager * p_pManager, wxWindow * parent, wxWindowID p_id);
		~NewIcosaedronDialog();
		
		real GetIcosaedronRadius()const;
		int GetNbSubdiv()const;
		String GetNbSubdivStr()const;

	private:
		virtual void _initialise();
		DECLARE_EVENT_TABLE()
	};
}

#endif
