/***************************************************************************
                         qgsprocessingwidgetwrapper.h
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGWIDGETWRAPPER_H
#define QGSPROCESSINGWIDGETWRAPPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>
#include <memory>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsprocessinggui.h"
#include "qgsvectorlayer.h"

class QgsProcessingParameterDefinition;
class QgsProcessingContext;
class QgsProcessingModelerParameterWidget;
class QgsProcessingModelAlgorithm;
class QLabel;
class QgsPropertyOverrideButton;
class QgsVectorLayer;
class QgsProcessingModelAlgorithm;
class QgsMapCanvas;
class QgsProcessingAlgorithm;

/**
 * \class QgsProcessingContextGenerator
 *
 * An interface for objects which can create Processing contexts.
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingContextGenerator
{
  public:

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return a Processing context.
     *
     * Note that ownership of the context is not transferred - it is intended that subclasses
     * return a pointer to a context which they have already created and own.
     */
    virtual QgsProcessingContext *processingContext() = 0;

    virtual ~QgsProcessingContextGenerator() = default;
};

/**
 * \ingroup gui
 * \class QgsProcessingParameterWidgetContext
 * Contains settings which reflect the context in which a Processing parameter widget is shown, e.g., the
 * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
 * to fine-tune its behavior.
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingParameterWidgetContext
{
  public:

    /**
     * Constructor for QgsProcessingParameterWidgetContext.
     */
    QgsProcessingParameterWidgetContext() = default;

    /**
     * Sets the map \a canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * \see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas associated with the widget.
     * \see setMapCanvas()
     */
    QgsMapCanvas *mapCanvas() const;

    /**
     * Sets the \a project associated with the widget. This allows the widget to retrieve the map layers
     * and other properties from the correct project.
     * \see project()
     * \since QGIS 3.8
     */
    void setProject( QgsProject *project );

    /**
     * Returns the project associated with the widget.
     * \see setProject()
     */
    QgsProject *project() const;

    /**
     * Returns the model which the parameter widget is associated with.
     *
     * \see setModel()
     * \see modelChildAlgorithmId()
     */
    QgsProcessingModelAlgorithm *model() const;

    /**
     * Sets the \a model which the parameter widget is associated with.
     *
     * \see model()
     * \see setModelChildAlgorithmId()
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the child algorithm ID within the model which the parameter widget is associated with.
     *
     * \see setModelChildAlgorithmId()
     * \see model()
     */
    QString modelChildAlgorithmId() const;

    /**
     * Sets the child algorithm \a id within the model which the parameter widget is associated with.
     *
     * \see modelChildAlgorithmId()
     * \see setModel()
     */
    void setModelChildAlgorithmId( const QString &id );

  private:

    QgsProcessingModelAlgorithm *mModel = nullptr;

    QString mModelChildAlgorithmId;

    QgsMapCanvas *mMapCanvas = nullptr;

    QgsProject *mProject = nullptr;

};

#ifndef SIP_RUN
///@cond PRIVATE
class GUI_EXPORT QgsProcessingGuiUtils
{
  public:

    static QgsExpressionContext createExpressionContext( QgsProcessingContextGenerator *processingContextGenerator = nullptr,
        const QgsProcessingParameterWidgetContext &widgetContext = QgsProcessingParameterWidgetContext(),
        const QgsProcessingAlgorithm *algorithm = nullptr,
        const QgsVectorLayer *linkedLayer = nullptr );


};
///@endcond
#endif

/**
 * \class QgsAbstractProcessingParameterWidgetWrapper
 *
 * A widget wrapper for Processing parameter value widgets.
 *
 * Widget wrappers are used to create widgets for individual Processing parameters, and
 * handle retrieving and setting the current value for those parameters.
 *
 * Widget wrappers can be created for different dialog types, allowing different
 * appearance and behavior of widgets depending on whether they are being created for
 * use in a standard algorithm dialog, a batch processing dialog, or a modeler dialog.
 *
 * Individual widget wrappers are not usually created directly, instead they are
 * constructed through the central registry, via calls to
 * QgsGui::processingGuiRegistry()->createParameterWidgetWrapper().
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsAbstractProcessingParameterWidgetWrapper : public QObject, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAbstractProcessingParameterWidgetWrapper, for the specified
     * \a parameter definition and dialog \a type.
     */
    QgsAbstractProcessingParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the dialog type for which widgets and labels will be created by this wrapper.
     */
    QgsProcessingGui::WidgetType type() const;

    /**
     * Sets the \a context in which the Processing parameter widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * Subclasses should take care to call the base class method when reimplementing this method.
     *
     * \see widgetContext()
     */
    virtual void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Returns the context in which the Processing parameter widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * \see setWidgetContext()
     */
    const QgsProcessingParameterWidgetContext &widgetContext() const;

    /**
     * Creates and return a new wrapped widget which allows customization of the parameter's value.
     *
     * The caller takes ownership of the returned widget. The wrapped widget can be retrieved at a later
     * stage by calling wrappedWidget().
     *
     * The supplied \a context is used while initializing the widget to the parameter's default value.
     *
     * \see createWrappedLabel()
     */
    QWidget *createWrappedWidget( QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates and returns a new label to accompany widgets created by the wrapper.
     *
     * The caller takes ownership of the returned label. Some parameter type and dialog type
     * combinations will return NULLPTR for this method. If NULLPTR is returned, then no
     * label should be shown for the parameter's widget (i.e. the label is embedded inside the
     * widget itself).
     *
     * The wrapped label can be retrieved at a later stage by calling wrappedLabel().
     *
     * \see createWrappedWidget()
     */
    QLabel *createWrappedLabel() SIP_FACTORY;

    /**
     * Returns the current wrapped widget, if any.
     * \see createWrappedWidget()
     */
    QWidget *wrappedWidget();

    /**
     * Returns the current wrapped label, if any.
     * \see createWrappedLabel()
     */
    QLabel *wrappedLabel();

    /**
     * Returns the parameter definition associated with this wrapper.
     */
    const QgsProcessingParameterDefinition *parameterDefinition() const;

    // TODO QGIS 4.0 -- remove
#ifdef SIP_RUN
    % Property( name = param, get = parameterDefinition )
#endif

    /**
     * Sets the current \a value for the parameter.
     *
     * The \a context argument is used to specify the wider Processing context which the
     * current value is associated with.
     *
     * \see parameterValue()
     */
    void setParameterValue( const QVariant &value, QgsProcessingContext &context );

    /**
     * Returns the current value of the parameter.
     *
     * \see setParameterValue()
     */
    QVariant parameterValue() const;

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the wrapper when required.
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

    /**
     * Called after all wrappers have been created within a particular dialog or context,
     * allowing the wrapper to connect to the wrappers of other, related parameters.
     */
    virtual void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers );

    QgsExpressionContext createExpressionContext() const override;

  signals:

    // TODO QGIS 4.0 - remove wrapper parameter - this is kept for compatibility with 3.x API,
    // yet can easily be retrieved by checking the sender()

    /**
     * Emitted whenever the parameter value (as defined by the wrapped widget) is changed.
     */
    void widgetValueHasChanged( QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  protected:

    /**
     * Creates a new widget which allows customization of the parameter's value.
     *
     * The caller takes ownership of the returned widget.
     *
     * \see createLabel()
     */
    virtual QWidget *createWidget() = 0 SIP_FACTORY;

    /**
     * Creates a new label to accompany widgets created by the wrapper.
     *
     * The caller takes ownership of the returned label. Some parameter type and dialog type
     * combinations will return NULLPTR for this method. If NULLPTR is returned, then no
     * label should be shown for the parameter's widget (i.e. the label is embedded inside the
     * widget itself).
     *
     * \see createWidget()
     */
    virtual QLabel *createLabel() SIP_FACTORY;

    /**
     * Sets the current \a value for the parameter to show in the widget.
     *
     * The \a context argument is used to specify the wider Processing context which the
     * current value is associated with.
     *
     * \see widgetValue()
     */
    virtual void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) = 0;

    /**
     * Returns the current value of the parameter.
     *
     * \see setWidgetValue()
     */
    virtual QVariant widgetValue() const = 0;

    /**
     * Returns the optional vector layer associated with this widget wrapper, or NULLPTR if no vector
     * layer is applicable.
     *
     * This is used to correctly generate expression contexts within the GUI, e.g. to allow expression
     * buttons and property override buttons to correctly show the appropriate vector layer fields.
     *
     * \since QGIS 3.6
     */
    virtual const QgsVectorLayer *linkedVectorLayer() const;

  protected:

    QgsProcessingContextGenerator *mProcessingContextGenerator = nullptr;
    QgsProcessingParameterWidgetContext mWidgetContext;

  private slots:

    void parentLayerChanged( QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  private:

    QgsProcessingGui::WidgetType mType = QgsProcessingGui::Standard;
    const QgsProcessingParameterDefinition *mParameterDefinition = nullptr;

    void setDynamicParentLayerParameter( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

    QPointer< QWidget > mWidget;
    QPointer< QgsPropertyOverrideButton > mPropertyButton;
    QPointer< QLabel > mLabel;
    std::unique_ptr< QgsVectorLayer > mDynamicLayer;

    friend class TestProcessingGui;

};


/**
 * \class QgsProcessingParameterWidgetFactoryInterface
 *
 * An interface for Processing widget wrapper factories.
 *
 * Widget wrapper factories allow creation of QgsAbstractProcessingParameterWidgetWrapper objects.
 * They are centrally managed by QgsProcessingGuiRegistry. Usually, individual factories
 * are not directly utilized, rather the QgsGui::processingGuiRegistry()->createParameterWidgetWrapper()
 * method is used to create widget wrappers.
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingParameterWidgetFactoryInterface
{

  public:

    virtual ~QgsProcessingParameterWidgetFactoryInterface() = default;

    /**
     * Returns the type string for the parameter type the factory is associated with.
     */
    virtual QString parameterType() const = 0;

    /**
     * Creates a new widget wrapper for the specified \a parameter definition.
     *
     * The \a type argument indicates the dialog type to create a wrapper for.
     *
     * \see createModelerWidgetWrapper()
     */
    virtual QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter,
        QgsProcessingGui::WidgetType type ) = 0 SIP_FACTORY;

    /**
     * Creates a new modeler parameter widget for the given \a parameter. This widget allows
     * configuration of the parameter's value when used inside a Processing \a model.
     *
     * The ID of the child algorithm within the model must be specified via the \a childId
     * argument. This value corresponds to the QgsProcessingModelChildAlgorithm::childId()
     * string, which uniquely identifies which child algorithm the parameter is associated
     * with inside the given \a model.
     *
     * A Processing \a context must also be specified, which allows the widget
     * to resolve parameter values which are context dependent. The context must
     * last for the lifetime of the widget.
     *
     * \see createWidgetWrapper()
     */
    virtual QgsProcessingModelerParameterWidget *createModelerWidgetWrapper( QgsProcessingModelAlgorithm *model,
        const QString &childId,
        const QgsProcessingParameterDefinition *parameter,
        QgsProcessingContext &context );

  protected:

    /**
     * Returns a list of compatible Processing parameter types for inputs
     * for this parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds to the
     * various available values for QgsProcessingParameterDefinition::type().
     *
     * Subclasses should return a list of all QgsProcessingParameterDefinition::type()
     * values which can be used as input values for the parameter.
     *
     * \see compatibleOutputTypes()
     * \see compatibleDataTypes()
     */
    virtual QStringList compatibleParameterTypes() const = 0;

    /**
     * Returns a list of compatible Processing output types for inputs
     * for this parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds to the
     * various available values for QgsProcessingOutputDefinition::type().
     *
     * Subclasses should return a list of all QgsProcessingOutputDefinition::type()
     * values which can be used as values for the parameter.
     *
     * \see compatibleParameterTypes()
     * \see compatibleDataTypes()
     */
    virtual QStringList compatibleOutputTypes() const = 0;

    /**
     * Returns a list of compatible Processing data types for inputs
     * for this parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds
     * to the various available values from QgsProcessing::SourceType.
     *
     * Subclasses should return a list of all QgsProcessing::SourceType
     * values which can be used as values for the parameter.
     *
     * \see compatibleParameterTypes()
     * \see compatibleOutputTypes()
     */
    virtual QList< int > compatibleDataTypes() const = 0;

    /**
     * Returns the expected expression format string for expression results for the parameter
     * within model child algorithms.
     *
     * This is shown in modeler widget wrappers when using the "pre-calculated" expression mode,
     * and should give helpful text to users to indicate the expected results from the expression.

     * This is purely a text format and no expression validation is made against it.
     */
    virtual QString modelerExpressionFormatString() const;

};

#endif // QGSPROCESSINGWIDGETWRAPPER_H
