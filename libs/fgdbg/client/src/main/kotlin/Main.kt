/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import androidx.compose.desktop.ui.tooling.preview.Preview
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.material.Button
import androidx.compose.material.ButtonDefaults
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Window
import androidx.compose.ui.window.application
import com.charleskorn.kaml.Yaml
import io.ktor.client.HttpClient
import io.ktor.client.plugins.websocket.WebSockets
import io.ktor.client.plugins.websocket.webSocket
import io.ktor.http.HttpMethod
import io.ktor.websocket.Frame
import io.ktor.websocket.readText
import kotlinx.coroutines.delay
import kotlinx.serialization.Serializable

private enum class ConnectionState { UNKNOWN, CONNECTED, CONNECTING, ERROR }

private data class Connection(
    val host: String = "localhost",
    val port: Int = 8082,
    val path: String = "/",
    val state: MutableState<ConnectionState> = mutableStateOf(ConnectionState.UNKNOWN),
    val reconnectDelay: Int = 5,
)

@Serializable
data class Payload(
    val passes: Map<Int, Pass>,
    val resources: Map<Int, Resource>,
)

@Serializable
data class Pass(
    val name: String,
    val reads: Set<Int>,
    val writes: Set<Int>,
)

@Serializable
data class Resource(
    val name: String,
    val size: Int,
)

fun parseMessage(message: String): Payload {
    println("Received payload: $message")
    try {
        return Yaml.default.decodeFromString(Payload.serializer(), message)
    } catch (e: Exception) {
        // TODO(@raviola): Better error handling
        println("Error parsing server payload: ${e.message}")
        throw e
    }
}

@Composable
@Preview
fun App() {
    val connection = remember { Connection() }
    val client = remember { HttpClient { install(WebSockets) } }
    var message by remember { mutableStateOf("") }
    val payload = remember { mutableStateOf(Payload(emptyMap(), emptyMap())) }

    LaunchedEffect(Unit) {
        while (true) {
            connection.state.value = ConnectionState.CONNECTING
            try {
                client.webSocket(HttpMethod.Get, connection.host, connection.port, connection.path) {
                    message = "Connected!"
                    connection.state.value = ConnectionState.CONNECTED
                    while (true) {
                        val frame = incoming.receive() as? Frame.Text ?: continue
                        payload.value = parseMessage(frame.readText())
                    }
                }
            } catch (e: Exception) {
                connection.state.value = ConnectionState.ERROR
                repeat(connection.reconnectDelay) {
                    val remaining = connection.reconnectDelay - it
                    message = "Connection interrupted, will re-connect in $remaining seconds..."
                    delay(1000)
                }
            }
        }
    }

    DisposableEffect(Unit) { onDispose { client.close() } }

    MaterialTheme {
        when (connection.state.value) {
            ConnectionState.CONNECTED -> {
                Column {
                    Text(text = "Listening on ${connection.host}:${connection.port} ")
                    Grid(payload.value)
                }
            }
            ConnectionState.ERROR -> Text(text = message, color = Color.Red)
            else -> Text(connection.state.value.name)
        }
    }
}

@Composable
fun Grid(payload: Payload) {
    val width = 120.dp
    val height = 40.dp
    Column {
        // HEADER
        Row(Modifier.height(height)) {
            Spacer(modifier = Modifier.width(width))
            for (pass in payload.passes.values) Button(modifier = Modifier.width(width), onClick = {}) {
                Text(pass.name)
            }
        }
        // ROWS
        for (resource in payload.resources) {
            Row(Modifier.height(height)) {
                Button(modifier = Modifier.width(width), onClick = {}) { Text(resource.value.name) }
                // INTERSECTIONS
                for (pass in payload.passes) {
                    val writes = pass.value.writes.contains(resource.key)
                    val reads = pass.value.reads.contains(resource.key)
                    val color = when {
                        writes && reads -> Color.Magenta
                        writes -> Color.Red
                        reads -> Color.Green
                        else -> Color.LightGray
                    }
                    Button(modifier = Modifier.width(width), colors = ButtonDefaults.buttonColors(backgroundColor = color), onClick = {}) {}
                }
            }
        }
    }
}

fun main() = application { Window(onCloseRequest = ::exitApplication) { App() } }
