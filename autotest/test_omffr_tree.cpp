/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test common source and utilities
 * Author:   Kyle Shannon <kyle@pobox.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include <boost/test/unit_test.hpp>

#include "omffr_tree.h"

/*
** OmffrTree Tests
** Tests: omffr_tree/creation_1
**                   creation_2
**                   creation_3
**                   external_nodes_1
**                   postorder_1
**                   postorder_2
**                   preorder_1
**                   creation_faulty
*/

struct OmffrTreeTestData
{
    OmffrTree<int>oTree;
    OmffrNode<int> *poNode;
};

BOOST_FIXTURE_TEST_SUITE( omffr_tree, OmffrTreeTestData )

/*
** Test empty creation.
*/
BOOST_AUTO_TEST_CASE( creation_1 )
{
    BOOST_CHECK_EQUAL( oTree.Size(), 0 );
}

BOOST_AUTO_TEST_CASE( creation_2 )
{
    oTree.AddChild( NULL, 1 );
    oTree.AddChild( oTree.Root(), 2 );
    oTree.AddChild( oTree.Root(), 3 );
    oTree.AddChild( oTree.Root(), 4 );
    oTree.AddChild( oTree.Root(), 5 );
    oTree.AddChild( oTree.Root(), 6 );
    BOOST_CHECK_EQUAL( oTree.Size(), 6 );
}

BOOST_AUTO_TEST_CASE( creation_3 )
{
    oTree.AddChild( NULL, 1 );
    poNode = oTree.AddChild( oTree.Root(), 2 );
    oTree.AddChild( poNode, 3 );
    oTree.AddChild( poNode, 4 );
    oTree.AddChild( poNode, 5 );
    oTree.AddChild( poNode, 6 );
    oTree.AddChild( poNode, 7 );
    BOOST_CHECK_EQUAL( oTree.Size(), 7 );
}

BOOST_AUTO_TEST_CASE( external_nodes_1 )
{
    oTree.AddChild( NULL, 1 );
    oTree.AddChild( oTree.Root(), 2 );
    oTree.AddChild( oTree.Root(), 3 );
    oTree.AddChild( oTree.Root(), 4 );
    poNode = oTree.AddChild( oTree.Root(), 5 );
    oTree.AddChild( poNode, 10 );
    std::vector<OmffrNode<int>*> aoExternal;
    aoExternal = oTree.GetExternalNodes( oTree.Root(), 1 );

    BOOST_CHECK_EQUAL( aoExternal.size(), 3 );
    aoExternal = oTree.GetExternalNodes( oTree.Root(), 2 );
    BOOST_CHECK_EQUAL( aoExternal.size(), 1 );
}

BOOST_AUTO_TEST_CASE( postorder_1 )
{
    oTree.AddChild( NULL, 1 );
    oTree.AddChild( oTree.Root(), 2 );
    oTree.AddChild( oTree.Root(), 3 );
    oTree.AddChild( oTree.Root(), 4 );
    oTree.AddChild( oTree.Root(), 5 );
    std::vector<OmffrNode<int>*>oNodes;
    oNodes = oTree.Postorder( oTree.Root() );
    std::vector<int>expected;
    expected.push_back( 2 );
    expected.push_back( 3 );
    expected.push_back( 4 );
    expected.push_back( 5 );
    expected.push_back( 1 );
    BOOST_CHECK_EQUAL( expected.size(), oNodes.size() );
    for( int i=0;i<expected.size();i++ )
    {
        BOOST_CHECK_EQUAL( expected[i], oTree.Element( oNodes[i] ) );
    }
}

BOOST_AUTO_TEST_CASE( postorder_2 )
{
    oTree.AddChild( NULL, 1 );
    oTree.AddChild( oTree.Root(), 2 );
    poNode = oTree.AddChild( oTree.Root(), 3 );
    oTree.AddChild( poNode, 4 );
    oTree.AddChild( poNode, 5 );
    oTree.AddChild( oTree.Root(  ), 6 );
    poNode = oTree.AddChild( oTree.Root(), 7 );
    oTree.AddChild( poNode, 8 );
    oTree.AddChild( poNode, 9 );
    std::vector<OmffrNode<int>*>aoNodes;
    aoNodes = oTree.Postorder( oTree.Root() );
    std::vector<int>expected;
    expected.push_back( 2 );
    expected.push_back( 4 );
    expected.push_back( 5 );
    expected.push_back( 3 );
    expected.push_back( 6 );
    expected.push_back( 8 );
    expected.push_back( 9 );
    expected.push_back( 7 );
    expected.push_back( 1 );
    BOOST_CHECK_EQUAL( expected.size(), aoNodes.size() );
    for( int i = 0; i < expected.size(); i++ )
    {
        BOOST_CHECK_EQUAL( expected[i], oTree.Element( aoNodes[i] ) );
    }
}

BOOST_AUTO_TEST_CASE( preorder_1  )
{
    oTree.AddChild( NULL, 1 );
    oTree.AddChild( oTree.Root(), 2 );
    poNode = oTree.AddChild( oTree.Root(), 3 );
    oTree.AddChild( poNode, 4 );
    oTree.AddChild( poNode, 5 );
    oTree.AddChild( oTree.Root(), 6 );
    poNode = oTree.AddChild( oTree.Root(), 7 );
    oTree.AddChild( poNode, 8 );
    oTree.AddChild( poNode, 9 );
    std::vector<OmffrNode<int>*>poNodes;
    poNodes = oTree.Preorder( oTree.Root() );
    std::vector<int>expected;
    expected.push_back( 1 );
    expected.push_back( 2 );
    expected.push_back( 3 );
    expected.push_back( 4 );
    expected.push_back( 5 );
    expected.push_back( 6 );
    expected.push_back( 7 );
    expected.push_back( 8 );
    expected.push_back( 9 );
    BOOST_CHECK_EQUAL(  expected.size(), poNodes.size() );
    for( int i=0;i<expected.size();i++ )
    {
        BOOST_CHECK_EQUAL( expected[i], oTree.Element( poNodes[i] ) );
    }
}

/*
** XXX
** Not so sure what this is actually for. Need to investigate.
** I don't even know how it passes.
** -kss 20131009
** XXX
*/
BOOST_AUTO_TEST_CASE( creation_faulty )
{
    oTree.AddChild( NULL, 10 );
    BOOST_CHECK_EQUAL( oTree.Size(), 1);
    oTree.AddChild(NULL, 10);
    BOOST_CHECK_EQUAL( oTree.Size(), 1);
}

BOOST_AUTO_TEST_SUITE_END() /* omffr_tree */

