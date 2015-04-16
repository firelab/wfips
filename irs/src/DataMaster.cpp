// Data Master - creates a master data set for CRunScenario that can be used for threading
// 2/15

#include<DataMaster.h>

// Default constructor
CDataMaster::CDataMaster() : m_VDispLogic(), m_VFWA(), m_VRescType(), m_VDispLoc(), m_VProdRates(), m_VResource(), m_VFire(), m_VResults(), m_DispTree(), m_DispMap()  
{
	m_NumDispLogic = 0;
	m_NumFWA = 0;
	m_NumRescType = 0;
	m_NumDispLoc = 0;
	m_NumProdRates = 0;
	m_NumResource = 0;
	m_NumFire = 0;
	m_NumResults = 0;
	m_EscapeIdx = -1;

        /* Preposition stuff */
	/*****ATTpp = PrepositionStruct();
	RCRWpp = PrepositionStruct();
	RHelpp = PrepositionStruct();
	FSCRWpp = PrepositionStruct();
	DOICRWpp = PrepositionStruct();
	FSENGpp = PrepositionStruct();
	DOIENGpp = PrepositionStruct();

	FSCRWdd = PrepositionStruct();
	DOICRWdd = PrepositionStruct();
	FSDZRdd = PrepositionStruct();
	DOIDZRdd = PrepositionStruct();
	FSENGdd = PrepositionStruct();
	DOIENGdd = PrepositionStruct();
	FSSEATdd = PrepositionStruct();
	DOISEATdd = PrepositionStruct();*/


}

// Destructor for CDataMaster
CDataMaster::~CDataMaster()
{
	// m_VResource is the only vector of pointers so need to delete all the CResource objects
	if (m_VResource.size() > 0)	{
		for (int i = 0; i < m_VResource.size(); i++)
			delete m_VResource[i];
	}

	//m_DispTree also needs to have the dispatchers deleted from it.
	if(m_DispTree.Size() > 0 && m_DispTree.Root() != NULL)
    {
       std::vector<OmffrNode<CDispatchBase*>*> nodes = m_DispTree.Preorder(m_DispTree.Root());
       for(int i = 0; i < nodes.size(); i++)	{
		   //Is the node a GACC Dispatcher?
		   CGACCDispatcher *GACCDispatch = dynamic_cast<CGACCDispatcher*>(m_DispTree.Element(nodes[i]));

		   if (GACCDispatch !=0)	{		// Is a GACC Dispatcher
			   // Get the vector of Type II IA Crews and iterate through each
			   vector<CTypeTwoIACrew> TypeTwoCrewVector = GACCDispatch->GetTypeTwoCrewVector();
			   for (int k = 0; k < TypeTwoCrewVector.size(); k++)	{
				   // Need to delete the IA crews do it in DataMaster
				   vector<CResource*> VIACrews = TypeTwoCrewVector[k].GetIACrewsVector();
				   for (int j = 0; j < VIACrews.size(); j++)
					delete VIACrews[j];
			   }
		   }

           delete m_DispTree.Element(nodes[i]);

		}
	}
}

//Create a Dispatcher tree from vectors in this class
bool CDataMaster::CreateDispTree()
{
	// Check that the dispatch location data has been loaded
	if (m_VDispLoc.size() == 0)
		return false;
	
	// Create a Dispatcher node pointer to contain returned values
	OmffrNode< CDispatchBase* > *Node;

	// Create a vector of tree nodes to contain returned values
	std::vector< OmffrNode< CDispatchBase* >* > VNodes;

	// Create the National Node for the Dispatcher Tree
	Node = m_DispTree.AddChild( NULL, new CNatDispatcher( "National" ));
	
	// Go through the Dispatch Location list and create nodes in the tree as needed
	for ( int j = 0; j < m_VDispLoc.size(); j++ )
	{
		// Get the FPU id for the dispatch location
		string FPUId = m_VDispLoc[j].GetFPU();
		
		// If the FPU id is not "NA_US" the dispatch location national dispatcher
		if ( FPUId == "NA_US" )	{

			// Get the National Node
			Node = m_DispTree.Root();

			// Get a pointer to the National Dispatcher so can add airtanker bases to it's vector
			CNatDispatcher *NatPtr = dynamic_cast< CNatDispatcher* >( m_DispTree.Element( Node ) );

			if (NatPtr != 0)    {
                // Does the Dispatch Location already exist in the tree?
			    VNodes = m_DispTree.Children( Node );

			    // If the node does not exist create it
			    bool Found = false;
			    for ( int i = 0; i < VNodes.size(); i++ )	{
				    // Get a pointer to the Dispatch Location Dispatcher
				    CDLDispatcher *DLDispatcher = dynamic_cast < CDLDispatcher * > ( m_DispTree.Element(VNodes[i]));
				    if ( DLDispatcher != 0 )	{

					    if ( DLDispatcher->GetDispLoc() == m_VDispLoc[j] )
						    Found = true;
				    }
			    }

			    if ( !Found )	{
				    string DLID = m_VDispLoc[j].GetDispLocID();
				    Node = m_DispTree.AddChild( Node, new CDLDispatcher( DLID, m_VDispLoc[j] ) );

				    // If the dispatch location is an airtanker base need to add it's pointer to the vector of airtanker bases at the National Node
				    string DispLocID = m_VDispLoc[j].GetDispLocID();
				    int Length = DispLocID.size();
				    if ( Length > 4 )	{
					    string Key = DispLocID.substr( Length-4, 4 );

					    if ( Key == "KYLE" && NatPtr != 0 )	{
						    CDLDispatcher *DLPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Node ) );
						    NatPtr->AddATDispatcher( DLPtr );
						
					    }

				    }
				
			    }
            }
		}

		else	{
			// Parse the FPU Id to determine the GACC Id 
			string GACCId = FPUId.substr(0,2);

			// Does the GACC node already exist in the tree
			Node = m_DispTree.Root();										// Get pointer to the root (National) Node
			VNodes = m_DispTree.Children( Node );							// Get a vector of the current GACC Nodes in the tree
			
			// If the node exists move to it, if not create it			
			bool Found = false;

			int NodeI = 0;
			for ( int i = 0; i < VNodes.size(); i++ )	{
				if ( m_DispTree.Element(VNodes[i])->GetDispatcherID() == GACCId )
				{
					Found = true;
					NodeI = i;
					break;
				}
					
			}

			if ( Found )
				Node = VNodes[NodeI];
			else
				Node = m_DispTree.AddChild( Node, new CGACCDispatcher( GACCId ) );

			// Is the Dispatch Location located at the GACC?
			if ( GACCId == FPUId )	{

				// Does the Dispatch Location already exist in the tree?
				VNodes = m_DispTree.Children( Node );

				// If the node does not exist create it
				Found = false;
				for ( int i = 0; i < VNodes.size(); i++ )	{
					// Get a pointer to the Dispatch Location Dispatcher
					CDLDispatcher *DLDispatcher = dynamic_cast < CDLDispatcher * > ( m_DispTree.Element(VNodes[i]));
					
                    if ( DLDispatcher != 0 )	{

						if ( DLDispatcher->GetDispLoc() == m_VDispLoc[j] )
						{
							Found = true;
							NodeI = i;
							break;
						}
					}
				}

				if ( !Found )	{
					string DLID = m_VDispLoc[j].GetDispLocID();
					Node = m_DispTree.AddChild( Node, new CDLDispatcher( DLID, m_VDispLoc[j] ) );
				}

			}

			else	{

				// Does the FPU node already exist in the tree?
				VNodes = m_DispTree.Children( Node );								// Get a vector of the current FPU Nodes in the tree

				// If the node exists move to it, if not create it
				Found = false;
				for ( int i = 0; i < VNodes.size(); i++ )	{

					if ( m_DispTree.Element(VNodes[i])->GetDispatcherID() == FPUId )
					{
						Found = true;
						NodeI = i;
						break;
					}
				}

				if ( Found )
					Node = VNodes[NodeI];
				else
					Node = m_DispTree.AddChild( Node, new CLocalDispatcher( FPUId ) );

				// Does the Dispatch Location already exist in the tree?
				VNodes = m_DispTree.Children( Node );

				// If the node does not exist create it
				Found = false;
				for ( int i = 0; i < VNodes.size(); i++ )	{
					// Get a pointer to the Dispatch Location Dispatcher
					CDLDispatcher *DLDispatcher = dynamic_cast < CDLDispatcher * > ( m_DispTree.Element(VNodes[i]));
					if ( DLDispatcher != 0 )	{

						if ( DLDispatcher->GetDispLoc() == m_VDispLoc[j] )
						{
							Found = true;
							NodeI = i;
							break;
						}
					}
				}

				if ( !Found )	{
					string DLID = m_VDispLoc[j].GetDispLocID();
					Node = m_DispTree.AddChild( Node, new CDLDispatcher( DLID, m_VDispLoc[j] ) );
				}
			}
		}
	}

	// Get the indexed map of the dispatch locations so can go directly to the desired node 
	// the index string is the dispatch location id
	m_DispMap = BuildDispatchIndex( &m_DispTree );
	m_InMap = BuildInternalIndex( &m_DispTree );

	// Call Function to add alternate dispatch locations for helicopters to deploy helitack
	AltHelicDLs( m_DispMap );

	return true;
}

// Build index for easy lookup of CDLDispatchers by dispatch location name
std::map<std::string, OmffrNode<CDispatchBase*>*> CDataMaster::BuildDispatchIndex(OmffrTree<CDispatchBase*>*oTree)
{
    std::map<std::string, OmffrNode<CDispatchBase*>*> map;
    std::vector<OmffrNode<CDispatchBase*>*> external;
    external = (*oTree).GetExternalNodes((*oTree).Root(), -1 );
    for( int i = 0; i < external.size(); i++ )
    {
        map.insert( std::pair<std::string,
                    OmffrNode<CDispatchBase*>*>( (*oTree).Element(external[i])->GetDispatcherID(), 
                                                 external[i] ) );
    }
    return map;
}

// Build a map with the ids for the interior nodes and their node for accessing the internal nodes directly 
std::map<std::string, OmffrNode<CDispatchBase*>*> CDataMaster::BuildInternalIndex( OmffrTree< CDispatchBase* > *oTree )
{
	// Get a vector with the internal nodes
	vector<OmffrNode<CDispatchBase*>*> InNodes = GetInternalNodes((*oTree).Root());

	// Create the map
	std::map<std::string, OmffrNode<CDispatchBase*>*> map;
	for (int i = 0; i < InNodes.size(); i++)	{

		map.insert(std::pair< std::string, OmffrNode<CDispatchBase*>*>((*oTree).Element(InNodes[i])->GetDispatcherID(), InNodes[i]));
	}
	return map;
}

// Get the internal nodes from Node down on the tree excluding Node
vector< OmffrNode<CDispatchBase*>*> CDataMaster::GetInternalNodes(OmffrNode<CDispatchBase*>* Node)
{
	// Get all the nodes past Node
	vector< OmffrNode<CDispatchBase*>*> AllNodes = m_DispTree.Postorder(Node);
	vector< OmffrNode<CDispatchBase*>*> InNodes;

	// If AllNodes is empty
	if (AllNodes.empty())
		return AllNodes;

	// Remove the external nodes from the AllNodes vector
	for (int i = 0; i < AllNodes.size(); i++)	{
		if (m_DispTree.Internal( AllNodes[i]))
			InNodes.push_back(AllNodes[i]);
	}

	return InNodes;
}

// Put pointers to the resources in the resource vector into the resource maps for the appropriate dispatch location dispatcher
bool CDataMaster::ResourcesToDispatchers()
{
	// Get a vector of all the dispatch locations (external nodes) in the tree
	std::vector<OmffrNode<CDispatchBase*>*> VDispatchLocations = m_DispTree.GetExternalNodes(m_DispTree.Root(), -1);
	
	// Iterate through the resources in the resource vector m_VResource
	for (int i = 0; i < m_VResource.size(); i++)
	{
		// If the resource is a large airtanker - resource type ATT add it to the list of airtankers at the national level
		if (m_VResource[i]->GetRescType().GetRescType() == "ATT")	{
			OmffrNode< CDispatchBase* > *Node = m_DispTree.Root();
			CNatDispatcher* NatDisp = dynamic_cast< CNatDispatcher* >(m_DispTree.Element( Node ));
			if ( NatDisp != 0 )
				NatDisp->AddAirtankerToVector(m_VResource[i]);
		}
		
		// Get the dispatch location for the resource
		CDispLoc DispLoc = m_VResource[i]->GetDispLoc();

		// Iterate through the vector for dispatch location dispatchers to find the dispatch location dispatcher the resource belongs in
		bool found = false;
		int j = 0;
		while (!found && j < VDispatchLocations.size())	{
			CDLDispatcher *DLDispLoc = dynamic_cast<CDLDispatcher*>(m_DispTree.Element( VDispatchLocations[j])); 
			if (DLDispLoc != 0)	{
				if(DispLoc == DLDispLoc->GetDispLoc())	{
					found = true;

					// Is this Dispatch Location Dispatcher at the GACC Level?
					OmffrNode<CDispatchBase*> *Node;
					Node = m_DispTree.Parent(VDispatchLocations[j]);
					CGACCDispatcher *GACCDisp = dynamic_cast<CGACCDispatcher*>(m_DispTree.Element(Node));
					if (GACCDisp != 0)	{

						// Check if resource is a Type II IA Crew 
						if (m_VResource[i]->GetRescType().GetRescType() == "CRW" && m_VResource[i]->GetStaffing() == 20)	{

							// create the CTypeTwoIACrew instance and put the 4-5 person IA Crews at the dispatch location
							GACCDisp->AddNewTypeTwoCrew(m_VResource[i], DLDispLoc);

						}

						else	
							DLDispLoc->InsertInRescMap(m_VResource[i]);
					}

					else
						DLDispLoc->InsertInRescMap(m_VResource[i]);

				}
			}

			else
				cout << "Have an external node in the dispatcher tree that is not a dispatch location dispatcher\n";

			j++;
		}

		if (!found)
			cout << "The dispatch location dispatcher for the resource is not found in the tree\n";
	}
	return true;
}

// Find the name of the closest airtanker base for each FWA in the analysis
bool CDataMaster::FWAsFindClosestAirtankerBases()
{
	// Get the Airtanker Base Dispatch Location Dispatchers
	vector<CDLDispatcher*> AirTankerBases;
	vector<OmffrNode< CDispatchBase*>*> VNodes = m_DispTree.Children(m_DispTree.Root());
	vector<OmffrNode< CDispatchBase*>*>::iterator Iter;

	// Iterate throught the nodes and find the airtanker bases
	for (Iter = VNodes.begin(); Iter != VNodes.end(); Iter++)	{
		// Get the Dispatch Location pointer, if it's not a dispatch location it's not an airtanker base
		CDLDispatcher* DispLocDispatcher = dynamic_cast< CDLDispatcher* >(m_DispTree.Element(*Iter));

		if (DispLocDispatcher != 0)	{

			// For now need to use the name to determine if it is an airtanker base.  WILL NEED TO CHANGE
			CDispLoc DispLoc = DispLocDispatcher->GetDispLoc();
			string DispLocID = DispLoc.GetDispLocID();

			int length = DispLocID.length();

			string key = "no";
			if (length > 4)
				key = DispLocID.substr(length-4, 4);
			

			// If this is an airtanker base add to the airtanker base vector
			if (key == "KYLE")
				AirTankerBases.push_back(DispLocDispatcher);

		}		
	}		// End iterate through children of national node
			
	// Iterate through each FWA and find the closest airtanker base
	for (int i = 0; i < m_VFWA.size(); i++)	{

		int ShortestDist = 1000000;
		string NameShortest = "";
		
		// Iterate throught all the Airtanker bases to find the closest airtanker base
		for (int a = 0; a < AirTankerBases.size(); a++)	{

			
			int Distance = m_VFWA[i].GetDistance(AirTankerBases[a]->GetDispLoc().GetDispLocID());

			if (Distance < ShortestDist && Distance > 0)	{

				// Save the information for the airtanker base
				NameShortest = AirTankerBases[a]->GetDispLoc().GetDispLocID();
				ShortestDist = Distance;

			}

		}	// End Search through Airtanker bases

		if (ShortestDist < 1000000)	
			m_VFWA[i].SetClosestATBase(NameShortest);

		else
            (void)ShortestDist;
			//cout << "Could not find a closest airtanker base for the FWA. \n";

	}	// End iterate through the FWAs

	return true;
}

// Create a map with the each FWA's id and it's indes in m_VFWA
bool CDataMaster::CreateFWAMap()
{
	for (int i = 0; i < m_VFWA.size(); i++)	{
		string FWAid = m_VFWA[i].GetFWAID();
		m_FWAMap.insert(std::pair<string, int>(FWAid, i));
	}
	return true;
}

// Assign alternate dispatch locations for helicopters to the nodes for dispatch location dispatchers
bool CDataMaster::AltHelicDLs( std::map<std::string, OmffrNode<CDispatchBase*>*> DLDispMap )
{
	// Use the dispatch map to assign alternate dispatch locations for helicopters to a dispatch location
	// Open the file containing the assignments
	ifstream inFile( "HeliBases.dat", ios::in );

	if ( !inFile )	{
		cout << "Could not open the file containing alternate helibases for helicopters to deploy helitack \n";
		return false;
	}

	else	{

		// Iterators to the Dispatch Location and Alternate dispatch location
		std::map< string, OmffrNode< CDispatchBase* >* >::iterator ItBase;
		std::map< string, OmffrNode< CDispatchBase* >* >::iterator ItAlt;

		// Read in the helicopter ID,  helitack crew ID pairs	
		string BaseID = "";
		string AltID = "";

		while 	( inFile >> BaseID >> AltID )	{
			
			// Check if both are good
			bool Good = true;

			ItBase = DLDispMap.find( BaseID );

			if ( ItBase == DLDispMap.end() )
				Good = false;

			ItAlt = DLDispMap.find( AltID );

			if ( ItAlt == DLDispMap.end() )
				Good = false;

			// If both values are good then assign the helitack crew to the helicotper
			if ( Good )	{

				CDLDispatcher* BasePtr = dynamic_cast< CDLDispatcher* >(  m_DispTree.Element( ItBase->second ) );
				CDLDispatcher* AltPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( ItAlt->second ) );

				if ( BasePtr != 0 && AltPtr != 0 )	
					BasePtr->AddtoAltHelicopterDLsList( AltPtr );

				else
					cout << "Did not get good DL pointers for adding alternate helicopter bases\n";

			}

			else
                            (void)Good;
				//cout << "Did not find dispatch locations to add alternate helicopter bases\n";

		}

	}

	inFile.close();
	return true;

}

// Make a copy of the data for a new thread
bool CDataMaster::CopyDataSet(std::vector<CResource*> &resourceVector, OmffrTree<CDispatchBase*> &newdispatchTree, std::map<string, OmffrNode<CDispatchBase*>*> &newDispMap, 
	std::map<string, OmffrNode<CDispatchBase*>*> &newInMap)
{
	std::map<CResource*, CResource*> rescMap;
	std::vector<CResource*> ATTPtrs;
	CopyResourceVector(resourceVector, rescMap, ATTPtrs);
	CopyDispatchTree(newdispatchTree,newDispMap, newInMap, rescMap, ATTPtrs);

	return true;
}

// Make a copy of the resource vector
bool CDataMaster::CopyResourceVector(std::vector<CResource*> &newResourceVector, std::map<CResource*, CResource*> &rescMap, vector<CResource*> &ATTPtrs)
{
	std::vector<CResource*> helicopters;
	newResourceVector.clear();
	rescMap.clear();

	for (int i = 0; i < m_VResource.size(); i++)	{

		string rescType = m_VResource[i]->GetRescType().GetRescType();

		if (rescType.substr(0,3) == "DZR" || rescType.substr(0,2) == "TP")
			newResourceVector.push_back(new CConstProd(*(dynamic_cast<CConstProd*>(m_VResource[i]))));
		if (rescType == "CRW")
			newResourceVector.push_back(new CCrew(*(dynamic_cast<CCrew*>(m_VResource[i]))));
		if (rescType.substr(0,2) == "EN")
			newResourceVector.push_back(new CEngine(*(dynamic_cast<CEngine*>(m_VResource[i]))));
		if (rescType == "WT")
			newResourceVector.push_back(new CWaterTender(*(dynamic_cast<CWaterTender*>(m_VResource[i]))));
		if (rescType == "ATT")	{
			newResourceVector.push_back(new CAirtanker(*(dynamic_cast<CAirtanker*>(m_VResource[i]))));
			int j = newResourceVector.size();
			ATTPtrs.push_back(newResourceVector[j-1]);
		}
		if (rescType == "SEAT" || rescType == "SCP")
			newResourceVector.push_back(new CSmallAT(*(dynamic_cast<CSmallAT*>(m_VResource[i]))));
		if (rescType == "SMJR")
			newResourceVector.push_back(new CSmokejumper(*(dynamic_cast<CSmokejumper*>(m_VResource[i]))));
		if (rescType == "SJAC")
			newResourceVector.push_back(new CSMJAircraft(*(dynamic_cast<CSMJAircraft*>(m_VResource[i]))));
		if (rescType == "HELI")	
			newResourceVector.push_back(new CHelitack(*(dynamic_cast<CHelitack*>(m_VResource[i]))));
		if (rescType.substr(0,3) == "HEL" && rescType != "HELI")	{	
			newResourceVector.push_back(new CHelicopter(*(dynamic_cast<CHelicopter*>(m_VResource[i]))));
			int j = newResourceVector.size();
			helicopters.push_back(newResourceVector[j-1]);
		}
		int j = newResourceVector.size();
		std::pair<CResource*, CResource*> pair(m_VResource[i], newResourceVector[j-1]);
		rescMap.insert(pair);
	}

	// Assign helitack crews to their primary helicopter
	CopyAttachedHelitackCrews(rescMap, helicopters);

	return true;
}

// Change attached crews for each helicopter because they are pointers
bool CDataMaster::CopyAttachedHelitackCrews(std::map<CResource*, CResource*> rescMap, std::vector<CResource*> helicopters)
{
	for (int i = 0; i < helicopters.size(); i++)	{

		CHelicopter* helic = dynamic_cast<CHelicopter*>(helicopters[i]);

		if (helic != 0)	{
			std::list<CResource*> oldPointers = helic->GetAttachedCrewsList();
			std::list<CResource*>::iterator it;
			std::list<CResource*> newPointers;
			std::map<CResource*, CResource*>::iterator itHeli;

			for (it = oldPointers.begin(); it != oldPointers.end(); it++)	{
				itHeli = rescMap.find(*it);
				if (itHeli != rescMap.end())
					newPointers.push_back((*itHeli).second);
			}
			helic->SetAttachedCrewsList(newPointers);
		}
	}
	return true;
}

// Make a copy of the dispatch tree
bool CDataMaster::CopyDispatchTree(OmffrTree<CDispatchBase*> &newTree, std::map<string, OmffrNode<CDispatchBase*>*> &newDispMap, 
	std::map<string, OmffrNode<CDispatchBase*>*> &newInMap, std::map<CResource*, CResource*> &rescMap, vector<CResource*> &ATTPtrs)
{
	if (!newTree.Empty())	{
		OmffrNode<CDispatchBase*>* Root = newTree.Root();
		newTree.RemoveTree(Root);
	}

	newDispMap.clear();
	newInMap.clear();

	std::vector<std::pair<CGACCDispatcher*, CResource*> > pairsGACCTypeTwoCrews;

	OmffrNode<CDispatchBase*>* RootNode = m_DispTree.Root();
	if (RootNode != 0)	{
		AddTreeNode(newTree, RootNode, NULL, newDispMap, newInMap, rescMap, ATTPtrs, pairsGACCTypeTwoCrews );
		return true;
	}

	// Put TypeTwoIACrews in their GACC DispatchersDispLoc;
	for (int i = 0; i < pairsGACCTypeTwoCrews.size(); i++)	{
		CGACCDispatcher* GACCdispatcher = pairsGACCTypeTwoCrews[i].first;
		CResource* typeTwoCrew = pairsGACCTypeTwoCrews[i].second;
		CDispLoc dispLoc = typeTwoCrew->GetDispLoc();
		string dispLocID = dispLoc.GetDispLocID();
		OmffrNode<CDispatchBase*>* DLNode = (*newDispMap.find(dispLocID)).second;
		CDispatchBase* dispatchbase = newTree.Element(DLNode);
		CDLDispatcher* DLdispatcher = dynamic_cast<CDLDispatcher*>(dispatchbase);
		if (DLdispatcher != 0)
			GACCdispatcher->AddNewTypeTwoCrew(typeTwoCrew, DLdispatcher);
	}

	return false;
}

// Determine the type of node in the dispatch tree
string CDataMaster::DetermineNodeType(OmffrNode<CDispatchBase*>* node)
{
	CNatDispatcher* Nat = dynamic_cast<CNatDispatcher*>(m_DispTree.Element(node));
	if (Nat != 0)	
		return "National";
	CGACCDispatcher *GACC = dynamic_cast<CGACCDispatcher*>(m_DispTree.Element(node));
	if (GACC != 0)
		return "GACC";
	CLocalDispatcher *Local = dynamic_cast<CLocalDispatcher*>(m_DispTree.Element(node));
	if (Local != 0)
		return "Local";
	CDLDispatcher *DL = dynamic_cast<CDLDispatcher*>(m_DispTree.Element(node));
	if (DL != 0)
		return "Dispatch Location";
	
	return "";
}

// Add a node and the tree below it to the to a copy of the tree
//	newTree is the tree being created, oldNode is the node being copied from the old tree, parentNode is the parent node for the node
//	being added to the new tree.
bool CDataMaster::AddTreeNode(OmffrTree<CDispatchBase*> &newTree, OmffrNode<CDispatchBase*>* oldNode, OmffrNode<CDispatchBase*>* parentNode, 
	std::map<string, OmffrNode<CDispatchBase*>*> &newDispMap, std::map<string, OmffrNode<CDispatchBase*>*> &newInMap, 
	std::map<CResource*, CResource*> &rescMap, vector<CResource*> &ATTPtrs, std::vector<std::pair<CGACCDispatcher*, CResource*> > pairsGACCTypeTwoCrews)
{
	bool status = true;
	OmffrNode<CDispatchBase*>* newNode;
	string dispatcherID = m_DispTree.Element(oldNode)->GetDispatcherID();
	vector<OmffrNode<CDispatchBase*>*> children;
	
	string nodeType = DetermineNodeType(oldNode);
	if (nodeType == "Dispatch Location")	{
		CDLDispatcher* DLdispatcher = dynamic_cast<CDLDispatcher*>(m_DispTree.Element(oldNode));
		if (DLdispatcher != 0)	{
			CDispLoc &dispLoc = DLdispatcher->GetDispLoc();
			newNode = newTree.AddChild(parentNode, new CDLDispatcher(dispatcherID, dispLoc));
			std::pair<string, OmffrNode<CDispatchBase*>*> pair(dispatcherID, newNode);
			newDispMap.insert(pair);
			// Add Resources to the rescMap
			CDLDispatcher* newDLdispatcher = dynamic_cast<CDLDispatcher*>(newTree.Element(newNode));
			if (newDLdispatcher != 0)	{
				std::multimap<string, CResource*> resources = DLdispatcher->GetRescMap();
				std::multimap<string, CResource*>::iterator rescIt;
				for (rescIt = resources.begin(); rescIt != resources.end(); rescIt++)	{
					string oldType = (*rescIt).first;
					CResource* oldPtr = (*rescIt).second;
					std::map<CResource*, CResource*>::iterator mapPtr = rescMap.find(oldPtr);
					if (mapPtr != rescMap.end())	{
						CResource* newPtr = (*mapPtr).second;
						if (newPtr !=0)
							newDLdispatcher->InsertInRescMap(newPtr);
					}
				}
			}
			int length = dispatcherID.size();
			if (length > 4)	{
				string key = dispatcherID.substr(length-4, 4);
				CNatDispatcher* Natdispatcher = dynamic_cast<CNatDispatcher*>(newTree.Element(parentNode));
				if (key == "KYLE" && Natdispatcher != 0)	{
					CDLDispatcher* newDLdispatcher = dynamic_cast<CDLDispatcher*>(newTree.Element(newNode));
					if (newDLdispatcher != 0)
						Natdispatcher->AddATDispatcher(newDLdispatcher);
				}
			}
		}
		return true;
	}

	if (nodeType == "National")	{
		newNode = newTree.AddChild(NULL, new CNatDispatcher(dispatcherID));
		std::pair<string, OmffrNode<CDispatchBase*>*> pair(dispatcherID, newNode);
		newInMap.insert(pair);
		CNatDispatcher* Natdispatcher = dynamic_cast<CNatDispatcher*>(newTree.Element(newNode));
		if (Natdispatcher != 0)	
			Natdispatcher->SetAirtankerVector(ATTPtrs);
	}

	if (nodeType == "GACC")	{
		newNode = newTree.AddChild(parentNode, new CGACCDispatcher(dispatcherID));
		std::pair<string, OmffrNode<CDispatchBase*>*> pair(dispatcherID, newNode);
		newInMap.insert(pair);
		CGACCDispatcher* GACCdispatcher = dynamic_cast<CGACCDispatcher*>(m_DispTree.Element(oldNode));
		if (GACCdispatcher != 0)	{
			std::vector<CTypeTwoIACrew> TypeTwoCrews = GACCdispatcher->GetTypeTwoCrewVector();
			for (int t = 0; t < TypeTwoCrews.size(); t++)	{
				CResource* rescPtr = TypeTwoCrews[t].GetResourcePtr();
				std::map<CResource*, CResource*>::iterator mapPtr = rescMap.find(rescPtr);
				if (mapPtr != rescMap.end())	{
					CResource* newRescPtr = (*mapPtr).second;
					CGACCDispatcher* newGACCdispatcher = dynamic_cast<CGACCDispatcher*>(newTree.Element(newNode));
					if (newGACCdispatcher != 0)	{
						std::pair<CGACCDispatcher*, CResource*> pair(GACCdispatcher, newRescPtr);
						pairsGACCTypeTwoCrews.push_back(pair);
					}
				}

				else return false;
			}
		}
	}

	if (nodeType == "Local")	{
		newNode = newTree.AddChild(parentNode, new CLocalDispatcher(dispatcherID));
		std::pair<string, OmffrNode<CDispatchBase*>*> pair(dispatcherID, newNode);
		newInMap.insert(pair);
	}
	children = m_DispTree.Children(oldNode);
	for (int i = 0; i < children.size(); i++)	{
		status = AddTreeNode(newTree, children[i], newNode, newDispMap, newInMap, rescMap, ATTPtrs, pairsGACCTypeTwoCrews);
		if (status == false)
			return false;
	}

	// Put alternate helicopter dispatch locations in their DLDispatchers
	// dispatcher levels
	// fix count
	

	return true;
}

	
	

	
