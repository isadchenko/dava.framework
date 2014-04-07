/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "TreeToAnimatedTreeConverter.h"
#include "Scene3D/Components/SpeedTreeComponents/SpeedTreeComponent.h"
#include "Render/Highlevel/SpeedTreeObject.h"

#include "Render/Material/NMaterialNames.h"

void TreeToAnimatedTreeConverter::ConvertTreesRecursive(Entity * node)
{
    for(int32 c = 0; c < node->GetChildrenCount(); ++c)
    {
        Entity * childNode = node->GetChild(c);
        ConvertTreesRecursive(childNode);
    }

    RenderComponent *rc = GetRenderComponent(node);
    if(!rc) return;

    RenderObject *ro = rc->GetRenderObject();
    if(!ro) return;
    
    bool isSpeedTree = false;

    uint32 count = ro->GetRenderBatchCount();
    for(uint32 b = 0; b < count; ++b)
    {
        RenderBatch *renderBatch = ro->GetRenderBatch(b);
        isSpeedTree |= (renderBatch->GetMaterial() && renderBatch->GetMaterial()->GetMaterialTemplate()->name == NMaterialName::SPEEDTREE_LEAF);
    }

    if(!isSpeedTree)
        return;

    SpeedTreeObject * treeObject = cast_if_equal<SpeedTreeObject*>(ro);
    if(!treeObject)
    {
        treeObject = new SpeedTreeObject();
        ro->Clone(treeObject);
        rc->SetRenderObject(treeObject);
        treeObject->Release();

        treeObject->RecalcBoundingBox();
    }

    ConvertForAnimations(treeObject);

    node->AddComponent(new SpeedTreeComponent());
}

void TreeToAnimatedTreeConverter::ConvertForAnimations(SpeedTreeObject * object)
{
    float32 treeHeight = object->GetBoundingBox().GetSize().z;

    uint32 size = object->GetRenderBatchCount();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = object->GetRenderBatch(k);
        PolygonGroup * pg = rb->GetPolygonGroup();
        if(pg)
        {
            int32 vertexFormat = pg->GetFormat();

            bool isLeaf = ((vertexFormat & EVF_TANGENT) > 0); //speedtree leaf batch
            int32 vxCount = pg->GetVertexCount();
            int32 indCount = pg->GetIndexCount();
            PolygonGroup * newPG = new PolygonGroup();

            DVASSERT((vertexFormat & EVF_BINORMAL) == 0);
            newPG->AllocateData(vertexFormat | EVF_BINORMAL, vxCount, indCount);

            //copy indicies
            for(int32 i = 0; i < indCount; ++i)
            {
                int32 index;
                pg->GetIndex(i, index);
                newPG->SetIndex(i, index);
            }

            //copy vertex data
            for(int32 i = 0; i < vxCount; ++i)
            {
                Vector3 vxPosition;
                uint32 color;
                Vector2 vxTx;

                pg->GetCoord(i, vxPosition);
                if((vertexFormat & EVF_COLOR) > 0)
                    pg->GetColor(i, color);
                if((vertexFormat & EVF_TEXCOORD0) > 0)
                    pg->GetTexcoord(0, i, vxTx);

                newPG->SetCoord(i, vxPosition);
                if((vertexFormat & EVF_COLOR) > 0)
                    newPG->SetColor(i, color);
                if((vertexFormat & EVF_TEXCOORD0) > 0)
                    newPG->SetTexcoord(0, i, vxTx);

                if(isLeaf)
                {
                    Vector3 vxTangent;
                    pg->GetTangent(i, vxTangent);
                    newPG->SetTangent(i, vxTangent);
                }

				float32 t0  = vxPosition.Length() * 100;
                float32 x = vxPosition.z / treeHeight;
                float32 flexebility = logf((expf(1.0) - 1) * x + 1);

                //Binormal: x: cos(T0);  y: sin(T0); z - flexebility
                Vector3 binormal(cosf(t0) * (.5f + x/2) / 100.f, sinf(t0) * (.5f + x/2) / 100.f, flexebility / 10.f);

                newPG->SetBinormal(i, binormal);
            }

            rb->SetPolygonGroup(newPG);
        }
    }
}
