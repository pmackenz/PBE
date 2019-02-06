/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS 
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, 
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written: fmckenna, adamzs

#include "ComponentContainer.h"
#include "Component.h"

#include <QPushButton>
#include <QScrollArea>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QDebug>
#include <sectiontitle.h>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QFileDialog>
#include <QScrollArea>


ComponentContainer::ComponentContainer(RandomVariablesContainer *theRV_IW, QWidget *parent)
    : SimCenterAppWidget(parent), theRandVariableIW(theRV_IW)
{
    verticalLayout = new QVBoxLayout();

    // title, add & remove button
    QHBoxLayout *titleLayout = new QHBoxLayout();

    SectionTitle *title=new SectionTitle();
    title->setText(tr("list of components"));
    title->setMinimumWidth(250);
    QSpacerItem *spacer1 = new QSpacerItem(50,10);
    QSpacerItem *spacer2 = new QSpacerItem(20,10);

    QPushButton *addComponent = new QPushButton();
    addComponent->setMinimumWidth(75);
    addComponent->setMaximumWidth(75);
    addComponent->setText(tr("add"));
    connect(addComponent,SIGNAL(clicked()),this,SLOT(addInputWidgetExistingComponent()));

    QPushButton *removeComponent = new QPushButton();
    removeComponent->setMinimumWidth(75);
    removeComponent->setMaximumWidth(75);
    removeComponent->setText(tr("remove"));
    connect(removeComponent,SIGNAL(clicked()),this,SLOT(removeInputWidgetExistingComponent()));

    titleLayout->addWidget(title);
    titleLayout->addItem(spacer1);
    titleLayout->addWidget(addComponent);
    titleLayout->addItem(spacer2);
    titleLayout->addWidget(removeComponent);
    titleLayout->addStretch();

    QScrollArea *sa = new QScrollArea;
    sa->setWidgetResizable(true);
    sa->setLineWidth(0);
    sa->setFrameShape(QFrame::NoFrame);

    QWidget *eventsWidget = new QWidget;

    eventLayout = new QVBoxLayout();
    eventLayout->addStretch();
    eventsWidget->setLayout(eventLayout);
    sa->setWidget(eventsWidget);


    verticalLayout->addLayout(titleLayout);
    verticalLayout->addWidget(sa);
    //verticalLayout->addStretch();

    this->setLayout(verticalLayout);
    connect(addComponent, SIGNAL(pressed()), this, SLOT(addComponent()));
    connect(removeComponent, SIGNAL(pressed()), this, SLOT(removeComponents()));
}


ComponentContainer::~ComponentContainer()
{

}


void ComponentContainer::addComponent(void)
{
   Component *theComponent = new Component();
   theComponents.append(theComponent);
   eventLayout->insertWidget(eventLayout->count()-1, theComponent);
   connect(this,SLOT(InputWidgetExistingComponentErrorMessage(QString)), theComponent, SIGNAL(sendErrorMessage(QString)));
}


void ComponentContainer::removeComponents(void)
{
    // find the ones selected & remove them
    int numInputWidgetExistingComponents = theComponents.size();
    for (int i = numInputWidgetExistingComponents-1; i >= 0; i--) {
      Component *theComponent = theComponents.at(i);
      if (theComponent->isSelectedForRemoval()) {
          theComponent->close();
          eventLayout->removeWidget(theComponent);
          theComponents.remove(i);
          theComponent->setParent(0);
          delete theComponent;
      }
    }
}


void
ComponentContainer::clear(void)
{
  // loop over random variables, removing from layout & deleting
  for (int i = 0; i <theComponents.size(); ++i) {
    Component *theComponent = theComponents.at(i);
    eventLayout->removeWidget(theComponent);
    delete theComponent;
  }
  theComponents.clear();
}


bool
ComponentContainer::outputToJSON(QJsonObject &jsonObject)
{
    //jsonObject["ComponentClassification"]="Earthquake";
    //jsonObject["type"] = "ComponentContainer";

    bool result = true;
    QJsonArray theArray;
    for (int i = 0; i <theComponents.size(); ++i) {
        QJsonObject rv;
        if (theComponents.at(i)->outputToJSON(rv)) {
            theArray.append(rv);

        } else {
            qDebug() << "OUTPUT FAILED" << theComponents.at(i)->getComponentName();
            result = false;
        }
    }
    jsonObject["Components"]=theArray;
    return result;
}

bool
ComponentContainer::inputFromJSON(QJsonObject &rvObj)
{
  bool result = true;

  // clean out current list
  this->clear();

  QJsonArray rvArray = rvObj["Components"].toArray();
  // foreach object in array
  foreach (const QJsonValue &rvValue, rvArray) {

      QJsonObject rvObject = rvValue.toObject();
      Component *theComponent = new Component();

      if (theComponent->inputFromJSON(rvObject)) { // this method is where type is set
          theComponents.append(theComponent);
          eventLayout->insertWidget(eventLayout->count()-1, theComponent);
      } else {
          result = false;
      }
  }

  return result;
}

bool 
ComponentContainer::copyFiles(QString &dirName) {
    return true;
}

void
ComponentContainer::errorMessage(QString message){
    emit sendErrorMessage(message);
}

