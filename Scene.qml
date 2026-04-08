import QtQuick

Item {
    id: gridBackground
    // Настройки сетки
    property int gridSize: Utils.objectsGridSize       // Шаг сетки
    property color gridColor: "#e0e0e0"
    property real lineWidth: 1.0

    Canvas {
        id: canvas
        anchors.fill: parent

        // Перерисовываем при изменении настроек
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.beginPath()
            ctx.strokeStyle = gridBackground.gridColor
            ctx.lineWidth = gridBackground.lineWidth;

            // Вертикальные линии
            for (var x = 0; x <= width; x += gridBackground.gridSize) {
                ctx.moveTo(x, 0)
                ctx.lineTo(x, height)

                var col = Math.floor(x / gridBackground.gridSize)
                ctx.fillText(col, x + 5, 15)
            }

            // Горизонтальные линии
            for (var y = 0; y <= height; y += gridBackground.gridSize) {
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)

                var row = Math.floor(y / gridBackground.gridSize)
                if (row > 0) { // Чтобы не накладывалось на 0 столбца
                    ctx.fillText(row, 5, y + 15)
                }
            }

            ctx.stroke()
        }
    }
}
