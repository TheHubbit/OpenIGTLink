/*=========================================================================
 
 Program:   OpenIGTLink Library
 Language:  C++
 Date:      $Date: 2016/02/12 19:53:38 $
 
 Copyright (c) Insight Software Consortium. All rights reserved.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "../Source/igtlPolyDataMessage.h"
#include <list>
#include "igtl_header.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "igtl_polydata.h"
#include "igtlOSUtil.h"
#include "../Testing/igtlutil/igtl_test_data_polydata.h"

#define POLY_BODY_SIZE 300

igtl::PolyDataMessage::Pointer polyDataMsg = igtl::PolyDataMessage::New();
igtl::PolyDataMessage::Pointer polyDataMsg2 = igtl::PolyDataMessage::New();

igtl::PolyDataPointArray::Pointer polyPoint = igtl::PolyDataPointArray::New();
igtl::PolyDataCellArray::Pointer polyGon = igtl::PolyDataCellArray::New();
igtl::PolyDataAttribute::Pointer polyAttr = igtl::PolyDataAttribute::New();

static igtl_float32 points[8][3]={{0,0,0}, {1,0,0}, {1,1,0}, {0,1,0},
  {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}};
static igtl_uint32 polyArray[6][4]={{0,1,2,3}, {4,5,6,7}, {0,1,5,4},
  {1,2,6,5}, {2,3,7,6}, {3,0,4,7}};
static igtl_float32 attribute[8]={0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
std::vector<std::list<igtl_uint32> >poly(6, std::list<igtl_uint32>(4));

void BuildUpElements()
{
  polyPoint->Clear();
  polyGon->Clear();
  for (int i =0; i<6; i++)
  {
    poly[i].assign(polyArray[i],polyArray[i]+4);
  }
  for (int i = 0; i < 8 ;i++)
  {
    polyPoint->AddPoint(points[i]);
  }
  for (int i = 0; i < 6 ;i++)
  {
    polyGon->AddCell(poly[i]);
  }
  polyAttr->SetType(IGTL_POLY_ATTR_TYPE_SCALAR);
  polyAttr->SetSize(8);
  polyAttr->SetName("attr");
  polyAttr->SetData(attribute);
  polyDataMsg = igtl::PolyDataMessage::New();
  polyDataMsg->SetPoints(polyPoint.GetPointer());
  polyDataMsg->SetPolygons(polyGon.GetPointer());
  polyDataMsg->AddAttribute(polyAttr.GetPointer());
  polyDataMsg->SetDeviceName("DeviceName");
  polyDataMsg->SetTimeStamp(0, 1234567890);
  polyDataMsg->Pack();
}

TEST(PolyDataMessageTest, Pack)
{
  BuildUpElements();
  for (int i = 0 ; i< 58;++i)
  {
    std::cerr<<+((igtlUint8*)polyDataMsg->GetPackPointer())[i]<<"  "<<+test_polydata_message_header[i]<<std::endl;
  }
  int r = memcmp((const void*)polyDataMsg->GetPackPointer(), (const void*)test_polydata_message_header,
                 (size_t)(IGTL_HEADER_SIZE));
  EXPECT_EQ(r, 0);
  r = memcmp((const void*)polyDataMsg->GetPackBodyPointer(), (const void*)test_polydata_message_body, POLY_BODY_SIZE);
  EXPECT_EQ(r, 0);
}


TEST(PolyDataMessageTest, Unpack)
{
  BuildUpElements();
  igtl::MessageHeader::Pointer headerMsg = igtl::MessageHeader::New();
  headerMsg->AllocatePack();
  memcpy(headerMsg->GetPackPointer(), (const void*)test_polydata_message_header, IGTL_HEADER_SIZE);
  headerMsg->Unpack();
  polyDataMsg2 = igtl::PolyDataMessage::New();
  polyDataMsg2->SetMessageHeader(headerMsg);
  polyDataMsg2->AllocatePack();
  igtl_header *messageHeader = (igtl_header *)polyDataMsg2->GetPackPointer();
  EXPECT_STREQ(messageHeader->device_name, "DeviceName");
  EXPECT_STREQ(messageHeader->name, "POLYDATA");
  EXPECT_EQ(messageHeader->version, 1);
  EXPECT_EQ(messageHeader->timestamp, 1234567890);
  EXPECT_EQ(messageHeader->body_size, POLY_BODY_SIZE);
  
  memcpy(polyDataMsg2->GetPackBodyPointer(), polyDataMsg->GetPackBodyPointer(), POLY_BODY_SIZE);
  polyDataMsg2->Unpack();
  
  igtl::PolyDataPointArray::Pointer pointUnpacked = polyDataMsg2->GetPoints();;
  igtl::PolyDataCellArray::Pointer polygonUnpacked = polyDataMsg2->GetPolygons();
  igtl::PolyDataAttribute::Pointer attrUnpacked = polyDataMsg2->GetAttribute(0);
  
  for (int i = 0; i<8; i++)
  {
    igtlFloat32 point[3] = {0,0,0};
    pointUnpacked->GetPoint(i, point);
    EXPECT_THAT(point, ::testing::ElementsAreArray(points[i]));
  }
  for (int i = 0; i<6; i++)
  {
    igtl_uint32 polygon[4] = {0,0,0,0};
    polygonUnpacked->GetCell(i, polygon);
    EXPECT_THAT(polygon, ::testing::ElementsAreArray(polyArray[i]));
  }
  igtl_float32 attributeValue[1];
  for (int i = 0; i<8; i++)
  {
    attrUnpacked->GetNthData(i, attributeValue);
    EXPECT_EQ(attributeValue[0], attribute[i]);
  }
}


int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

