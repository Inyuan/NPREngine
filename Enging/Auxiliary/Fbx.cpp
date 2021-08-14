#include "Fbx.h"
#include <algorithm>

using namespace DirectX;

/* Tab character ("\t") counter */
int numTabs = 0;

/**
 * Print the required number of tabs.
 */
void PrintTabs() {
    for (int i = 0; i < numTabs; i++)
        printf("\t");
}

/**
 * Return a string-based representation based on the attribute type.
 */
FbxString Fbx::GetAttributeTypeName(FbxNodeAttribute::EType type) {
    switch (type) {
    case FbxNodeAttribute::eUnknown: return "unidentified";
    case FbxNodeAttribute::eNull: return "null";
    case FbxNodeAttribute::eMarker: return "marker";
    case FbxNodeAttribute::eSkeleton: return "skeleton";
    case FbxNodeAttribute::eMesh: return "mesh";
    case FbxNodeAttribute::eNurbs: return "nurbs";
    case FbxNodeAttribute::ePatch: return "patch";
    case FbxNodeAttribute::eCamera: return "camera";
    case FbxNodeAttribute::eCameraStereo: return "stereo";
    case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
    case FbxNodeAttribute::eLight: return "light";
    case FbxNodeAttribute::eOpticalReference: return "optical reference";
    case FbxNodeAttribute::eOpticalMarker: return "marker";
    case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
    case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
    case FbxNodeAttribute::eBoundary: return "boundary";
    case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
    case FbxNodeAttribute::eShape: return "shape";
    case FbxNodeAttribute::eLODGroup: return "lodgroup";
    case FbxNodeAttribute::eSubDiv: return "subdiv";
    default: return "unknown";
    }
}

/**
 * Print an attribute.
 */
void Fbx::PrintAttribute(FbxNodeAttribute* pAttribute) {
    if (!pAttribute) return;

    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();
    PrintTabs();
    // Note: to retrieve the character array of a FbxString, use its Buffer() method.
    printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

/**
 * Print a node, its attributes, and all its children recursively.
 */
void Fbx::PrintNode(FbxNode* pNode) 
{
    PrintTabs();
    const char* nodeName = pNode->GetName();
    FbxDouble3 translation = pNode->LclTranslation.Get();
    FbxDouble3 rotation = pNode->LclRotation.Get();
    FbxDouble3 scaling = pNode->LclScaling.Get();

    // Print the contents of the node.
    printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
        nodeName,
        translation[0], translation[1], translation[2],
        rotation[0], rotation[1], rotation[2],
        scaling[0], scaling[1], scaling[2]
    );
    numTabs++;

    // Print the node's attributes.
    for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
        PrintAttribute(pNode->GetNodeAttributeByIndex(i));

    // Recursively print the children.
    for (int j = 0; j < pNode->GetChildCount(); j++)
        PrintNode(pNode->GetChild(j));

    numTabs--;
    PrintTabs();
    printf("</node>\n");
}

void Fbx::PrintFbx()
{
    // Print the nodes of the scene and their attributes recursively.
    // Note that we are not printing the root node because it should
    // not contain any attributes.
    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode) {
        for (int i = 0; i < lRootNode->GetChildCount(); i++)
            PrintNode(lRootNode->GetChild(i));
    }
    printf("end.\n");
}

/**
 * Main function - loads the hard-coded fbx file,
 * and prints its contents in an xml format to stdout.
 */
bool Fbx::ReadFbx(const char* lFilename)
{
    
    // Destroy the SDK manager and all the other objects it was handling.
    //if(lSdkManager)
        //lSdkManager->Destroy();
    // Initialize the SDK manager. This object handles all our memory management.
    lSdkManager = FbxManager::Create();

    // Create the IO settings object.
    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    // Create an importer using the SDK manager.
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

    // Use the first argument as the filename for the importer.
    if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) 
    {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        return false;//exit(-1);
    }

    // Create a new scene so that it can be populated by the imported file.
    lScene = FbxScene::Create(lSdkManager, "myScene");

    // Import the contents of the file into the scene.
    lImporter->Import(lScene);

    // The file is imported; so get rid of the importer.
    lImporter->Destroy();

    FbxNode* lRootNode = lScene->GetRootNode();
    ProcessNode(lRootNode);
    return true;
}


void Fbx::ProcessNode(FbxNode* pNode)
{
    FbxNodeAttribute::EType attributeType;
    
    FbxNodeAttribute* NA = pNode->GetNodeAttribute();
    
    if(NA)
    {
        switch (NA->GetAttributeType())
        {
        case FbxNodeAttribute::eMesh:
            ProcessMesh(pNode);
            break;
        case FbxNodeAttribute::eSkeleton:
            ProcessSkeleton(pNode);
            break;
        case FbxNodeAttribute::eLight:
            ProcessLight(pNode);
            break;
        case FbxNodeAttribute::eCamera:
            ProcessCamera();
            break;
        }

    }

    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        ProcessNode(pNode->GetChild(i));
    }

}

bool Fbx::ProcessCamera()
{
    return true;
}
bool Fbx::ProcessLight(FbxNode* pNode)
{
    return true;
}
bool Fbx::ProcessSkeleton(FbxNode* pNode)
{
    return true;
}

bool Fbx::ProcessMesh(FbxNode* pNode)
{
    pMesh = pNode->GetMesh();
    if (pMesh == NULL)
    {
        return false;
    }

    return true;
}


void Fbx::ReadVertex(int ctrlPointIndex, DirectX::XMFLOAT3* pVertex)
{
    FbxVector4* pCtrlPoint = pMesh->GetControlPoints();

    pVertex->x = pCtrlPoint[ctrlPointIndex][0];
    pVertex->y = pCtrlPoint[ctrlPointIndex][1];
    pVertex->z = pCtrlPoint[ctrlPointIndex][2];
}

void Fbx::ReadColor(int ctrlPointIndex, int vertexCounter, DirectX::XMFLOAT4* pColor)
{
    if (pMesh->GetElementVertexColorCount() < 1)
    {
        return;
    }

    FbxGeometryElementVertexColor* pVertexColor = pMesh->GetElementVertexColor(0);
    switch (pVertexColor->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
    {
        switch (pVertexColor->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pColor->x = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mRed;
            pColor->y = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mGreen;
            pColor->z = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mBlue;
            pColor->w = pVertexColor->GetDirectArray().GetAt(ctrlPointIndex).mAlpha;
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            int id = pVertexColor->GetIndexArray().GetAt(ctrlPointIndex);
            pColor->x = pVertexColor->GetDirectArray().GetAt(id).mRed;
            pColor->y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
            pColor->z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
            pColor->w = pVertexColor->GetDirectArray().GetAt(id).mAlpha;
        }
        break;

        default:
            break;
        }
    }
    break;

    case FbxGeometryElement::eByPolygonVertex:
    {
        switch (pVertexColor->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pColor->x = pVertexColor->GetDirectArray().GetAt(vertexCounter).mRed;
            pColor->y = pVertexColor->GetDirectArray().GetAt(vertexCounter).mGreen;
            pColor->z = pVertexColor->GetDirectArray().GetAt(vertexCounter).mBlue;
            pColor->w = pVertexColor->GetDirectArray().GetAt(vertexCounter).mAlpha;
        }
        break;
        case FbxGeometryElement::eIndexToDirect:
        {
            int id = pVertexColor->GetIndexArray().GetAt(vertexCounter);
            pColor->x = pVertexColor->GetDirectArray().GetAt(id).mRed;
            pColor->y = pVertexColor->GetDirectArray().GetAt(id).mGreen;
            pColor->z = pVertexColor->GetDirectArray().GetAt(id).mBlue;
            pColor->w = pVertexColor->GetDirectArray().GetAt(id).mAlpha;
        }
        break;
        default:
            break;
        }
    }
    break;
    }
}

bool Fbx::ReadUV(int ctrlPointIndex, int textureUVIndex, int uvLayer, DirectX::XMFLOAT2* pUV)
{
    if (uvLayer >= 2 || pMesh->GetElementUVCount() <= uvLayer)
    {
        return false;
    }

    FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV(uvLayer);

    switch (pVertexUV->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
    {
        switch (pVertexUV->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pUV->x = pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0];
            pUV->y = pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1];
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            int id = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);
            pUV->x = pVertexUV->GetDirectArray().GetAt(id)[0];
            pUV->y = pVertexUV->GetDirectArray().GetAt(id)[1];
        }
        break;

        default:
            break;
        }
    }
    break;

    case FbxGeometryElement::eByPolygonVertex:
    {
        switch (pVertexUV->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        case FbxGeometryElement::eIndexToDirect:
        {
            pUV->x = pVertexUV->GetDirectArray().GetAt(textureUVIndex)[0];
            pUV->y = pVertexUV->GetDirectArray().GetAt(textureUVIndex)[1];
        }
        break;

        default:
            break;
        }
    }
    break;
    }
}

void Fbx::ReadNormal(int ctrlPointIndex, int vertexCounter, DirectX::XMFLOAT3* pNormal)
{
    if (pMesh->GetElementNormalCount() < 1)
    {
        return;
    }

    FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(0);
    switch (leNormal->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
    {
        switch (leNormal->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pNormal->x = leNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];
            pNormal->y = leNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];
            pNormal->z = leNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            int id = leNormal->GetIndexArray().GetAt(ctrlPointIndex);
            pNormal->x = leNormal->GetDirectArray().GetAt(id)[0];
            pNormal->y = leNormal->GetDirectArray().GetAt(id)[1];
            pNormal->z = leNormal->GetDirectArray().GetAt(id)[2];
        }
        break;

        default:
            break;
        }
    }
    break;

    case FbxGeometryElement::eByPolygonVertex:
    {
        switch (leNormal->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pNormal->x = leNormal->GetDirectArray().GetAt(vertexCounter)[0];
            pNormal->y = leNormal->GetDirectArray().GetAt(vertexCounter)[1];
            pNormal->z = leNormal->GetDirectArray().GetAt(vertexCounter)[2];
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            int id = leNormal->GetIndexArray().GetAt(vertexCounter);
            pNormal->x = leNormal->GetDirectArray().GetAt(id)[0];
            pNormal->y = leNormal->GetDirectArray().GetAt(id)[1];
            pNormal->z = leNormal->GetDirectArray().GetAt(id)[2];
        }
        break;

        default:
            break;
        }
    }
    break;
    }
}

void Fbx::ReadTangent(int ctrlPointIndex, int vertecCounter, DirectX::XMFLOAT3* pTangent)
{
    if (pMesh->GetElementTangentCount() < 1)
    {
        return;
    }

    FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(0);

    switch (leTangent->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
    {
        switch (leTangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pTangent->x = leTangent->GetDirectArray().GetAt(ctrlPointIndex)[0];
            pTangent->y = leTangent->GetDirectArray().GetAt(ctrlPointIndex)[1];
            pTangent->z = leTangent->GetDirectArray().GetAt(ctrlPointIndex)[2];
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            int id = leTangent->GetIndexArray().GetAt(ctrlPointIndex);
            pTangent->x = leTangent->GetDirectArray().GetAt(id)[0];
            pTangent->y = leTangent->GetDirectArray().GetAt(id)[1];
            pTangent->z = leTangent->GetDirectArray().GetAt(id)[2];
        }
        break;

        default:
            break;
        }
    }
    break;

    case FbxGeometryElement::eByPolygonVertex:
    {
        switch (leTangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect:
        {
            pTangent->x = leTangent->GetDirectArray().GetAt(vertecCounter)[0];
            pTangent->y = leTangent->GetDirectArray().GetAt(vertecCounter)[1];
            pTangent->z = leTangent->GetDirectArray().GetAt(vertecCounter)[2];
        }
        break;

        case FbxGeometryElement::eIndexToDirect:
        {
            int id = leTangent->GetIndexArray().GetAt(vertecCounter);
            pTangent->x = leTangent->GetDirectArray().GetAt(id)[0];
            pTangent->y = leTangent->GetDirectArray().GetAt(id)[1];
            pTangent->z = leTangent->GetDirectArray().GetAt(id)[2];
        }
        break;

        default:
            break;
        }
    }
    break;
    }
}

