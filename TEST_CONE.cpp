#include "darkgdk.h"
#include "TEST_CONE.h"

using namespace std;

bool CreateNewObject            ( int iID, LPSTR pName, int iFrame );
bool SetupStandardVertex        ( DWORD dwFVF, BYTE* pVertex, int iOffset, float x, float y, float z, float nx, float ny, float nz, DWORD dwDiffuseColour, float tu, float tv );
bool SetupMeshFVFData           ( sMesh* pMesh, DWORD dwFVF, DWORD dwVertexCount, DWORD dwIndexCount );
bool SetNewObjectFinalProperties( int iID, float fRadius );
void SetTexture                 ( int iID, int iImage );


TEST_CONE::TEST_CONE(void)
{
	    // make a cone
		int iID; float fSize;

		iID = 1000; fSize = 50;

        // create a new object
        CreateNewObject ( iID, "cone", 1 );

        float fHeight   = fSize;
        int   iSegments = 11;

        // correct cone size
        fSize/=2.0f;
        
        // setup general object data
        sObject*        pObject = dbGetObject ( iID );
        sFrame*         pFrame  = pObject->pFrame;
        sMesh*          pMesh   = pObject->pFrame->pMesh;

        // create vrtex memory
        DWORD dwVertexCount = (iSegments * 2) + 1;                                              // store number of vertices
        DWORD dwIndexCount  = iSegments * 3;                                                    // store number of indices
        SetupMeshFVFData ( pMesh, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, dwVertexCount, dwIndexCount );

        float fDeltaSegAngle = ( (2.0f * D3DX_PI) / iSegments );
        float fSegmentLength = 1.0f / ( float ) iSegments;
        float fy0            = ( 90.0f - ( float ) D3DXToDegree ( atan ( fHeight / fSize ) ) ) / 90.0f;
        int       iVertex        = 0;
        int       iIndex         = 0;
        WORD  wVertexIndex   = 0;

        // for each segment, add a triangle to the sides triangle list
        for ( int iCurrentSegment = 0; iCurrentSegment <= iSegments; iCurrentSegment++ )
        {
                float x0 = fSize * sinf ( iCurrentSegment * fDeltaSegAngle );
                float z0 = fSize * cosf ( iCurrentSegment * fDeltaSegAngle );

                // Calculate normal
                D3DXVECTOR3 Normal = D3DXVECTOR3 ( x0, fy0, z0 );
                D3DXVec3Normalize ( &Normal, &Normal ); 

                // not the last segment though
                if ( iCurrentSegment < iSegments )
                {
                        // set vertex A
                        SetupStandardVertex ( pMesh->dwFVF,     pMesh->pVertexData,  iVertex, 0.0f, 0.0f+(fHeight/2.0f), 0.0f, Normal.x, Normal.y, Normal.z, D3DCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f - ( fSegmentLength * ( float ) iCurrentSegment ), 0.0f );

                        // increment vertex index
                        iVertex++;

                        // set vertex B
                        SetupStandardVertex ( pMesh->dwFVF,     pMesh->pVertexData,  iVertex, x0, 0.0f-(fHeight/2.0f), z0, Normal.x, Normal.y, Normal.z, D3DCOLOR_ARGB ( 255, 255, 255, 255 ), 1.0f - ( fSegmentLength * ( float ) iCurrentSegment ), 1.0f );

                        // increment vertex index
                        iVertex++;
                }
                else
                {
                        // set last vertex
                        SetupStandardVertex ( pMesh->dwFVF,     pMesh->pVertexData,  iVertex, x0, 0.0f - ( fHeight / 2.0f ), z0, Normal.x, Normal.y, Normal.z, D3DCOLOR_ARGB ( 255, 255, 255, 255 ), 0.0f, 1.0f );

                        // increment vertex index
                        iVertex++;
                }

                // not the last segment though
                if ( iCurrentSegment < iSegments )
                {
                        // set three indices per segment
                        pMesh->pIndices [ iIndex ] = wVertexIndex;
                        iIndex++;
                        wVertexIndex++;
                        
                        pMesh->pIndices [ iIndex ] = wVertexIndex;
                        iIndex++;

                        if ( iCurrentSegment == iSegments-1 )
                                wVertexIndex += 1;
                        else
                                wVertexIndex += 2;
                        
                        pMesh->pIndices [ iIndex ] = wVertexIndex;
                        iIndex++;
                        wVertexIndex--; 
                }
        }

        // setup mesh drawing properties
        pMesh->iPrimitiveType   = D3DPT_TRIANGLELIST;
        pMesh->iDrawVertexCount = ( iSegments * 2 ) + 1;
        pMesh->iDrawPrimitives  = iSegments;
        
        // setup new object and introduce to buffers
        SetNewObjectFinalProperties ( iID, fSize );

        // give the object a default texture
        SetTexture ( iID, 0 );

}

TEST_CONE::~TEST_CONE(void)
{
	if (dbObjectExist(1000)) dbDeleteObject(1000);
}
