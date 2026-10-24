/*
 * Audacity: A Digital Audio Editor
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

import Muse.UiComponents

import Audacity.Spectrogram 1.0

TrackSpectrogramBaseSection {
    id: root

    title: qsTrc("appshell/preferences/spectrogram", "Colors")

    required property TrackSpectrogramSettingsModel settingsModel

    Column {
        id: mainColumn

        spacing: root.narrowSpacing

        Repeater {
            id: repeater

            model: ColorSectionParameterListModel {
                settingsModel: root.settingsModel
                columnWidth: root.prefsColumnWidth
            }

            Row {
                spacing: 0

                StyledTextLabel {
                    width: root.mediumControlWidth
                    anchors.verticalCenter: parent.verticalCenter

                    text: colorControlLabel
                    elide: Text.ElideNone
                    wrapMode: Text.NoWrap
                    horizontalAlignment: Text.AlignLeft
                }

                IncrementalPropertyControl {
                    width: root.mediumControlWidth

                    navigation.panel: root.navigation
                    navigation.order: index
                    navigation.name: "ColorsIncrementalControl_" + index

                    minValue: colorControlMinValue
                    maxValue: colorControlMaxValue
                    measureUnitsSymbol: colorControlUnits
                    decimals: 0
                    step: 1

                    currentValue: colorControlCurrentValue
                    onValueEditingFinished: function (newValue) {
                        colorControlCurrentValue = newValue
                    }
                }
            }
        }

        Row {
            spacing: 0

            StyledTextLabel {
                width: root.mediumControlWidth
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("spectrogram/settings", "Scheme")
                elide: Text.ElideNone
                wrapMode: Text.NoWrap
                horizontalAlignment: Text.AlignLeft
            }

            StyledDropdown {
                width: root.largeControlWidth

                navigation.panel: root.navigation
                navigation.name: "ColorsSchemeComboBox"
                navigation.order: repeater.count

                model: settingsModel.colorSchemeNames
                currentIndex: settingsModel.colorScheme
                onActivated: function (index, value) {
                    settingsModel.colorScheme = index
                }
            }
        }

        CheckBox {
            id: combinedStereoCheckBox

            text: qsTrc("spectrogram/settings", "Combine stereo channels by balance")
            checked: settingsModel.stereoCombined

            navigation.panel: root.navigation
            navigation.name: "ColorsCombinedStereoCheckBox"
            navigation.order: repeater.count + 1

            onClicked: settingsModel.stereoCombined = checked
        }

        Row {
            spacing: 0
            visible: combinedStereoCheckBox.checked

            StyledTextLabel {
                width: root.mediumControlWidth
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("spectrogram/settings", "Balance contrast")
                elide: Text.ElideNone
                wrapMode: Text.NoWrap
                horizontalAlignment: Text.AlignLeft
            }

            IncrementalPropertyControl {
                width: root.mediumControlWidth

                navigation.panel: root.navigation
                navigation.name: "ColorsStereoBalanceGainControl"
                navigation.order: repeater.count + 2

                minValue: 0.25
                maxValue: 12
                decimals: 2
                step: 0.25
                currentValue: settingsModel.stereoBalanceGain
                onValueEditingFinished: function (newValue) {
                    settingsModel.stereoBalanceGain = newValue
                }
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: combinedStereoCheckBox.checked
            text: qsTrc("spectrogram/settings", "Brightness shows mean power. Red: left, purple: equal, blue: right.")
            wrapMode: Text.WordWrap
            color: ui.theme.fontSecondaryColor
        }
    }
}
